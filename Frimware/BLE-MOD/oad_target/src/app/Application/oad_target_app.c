/******************************************************************************

 @file  oad_target_app.c

 @brief This file contains the OAD Target App sample application for use with
        the CC2650 Bluetooth Low Energy Protocol Stack.

 Group: CMCU, SCS
 Target Device: CC2640R2

 ******************************************************************************
 
 Copyright (c) 2014-2017, Texas Instruments Incorporated
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 *  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.

 *  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.

 *  Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 ******************************************************************************
 Release Name: simplelink_cc2640r2_sdk_1_30_00_25
 Release Date: 2017-03-02 20:08:31
 *****************************************************************************/

/*********************************************************************
 * INCLUDES
 */
#include <string.h>

#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Queue.h>

#include "hci_tl.h"
#include "gatt.h"

#include "gapgattserver.h"
#include "gattservapp.h"

#include "oad_target.h"
#include "oad.h"
#include "ll_common.h"

#include "peripheral.h"
#include "gapbondmgr.h"

#include "osal_snv.h"
#include "icall_apimsg.h"

#include "util.h"
#include <ti/display/Display.h>
#include "board_key.h"
#include "board.h"

#include "icall_api.h"

#ifdef USE_RCOSC
 #include "rcosc_calibration.h" 
#endif //USE_RCOSC 
/*********************************************************************
 * CONSTANTS
 */
// Advertising interval when device is discoverable (units of 625us, 160=100ms)
#define DEFAULT_ADVERTISING_INTERVAL          160

// Limited discoverable mode advertises for 30.72s, and then stops
// General discoverable mode advertises indefinitely
#define DEFAULT_DISCOVERABLE_MODE             GAP_ADTYPE_FLAGS_GENERAL

// Minimum connection interval (units of 1.25ms, 12=15ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MIN_CONN_INTERVAL     12

// Maximum connection interval (units of 1.25ms, 12=15ms) if automatic
// parameter update request is enabled
#define DEFAULT_DESIRED_MAX_CONN_INTERVAL     12

// Slave latency to use if automatic parameter update request is enabled
#define DEFAULT_DESIRED_SLAVE_LATENCY         0

// Supervision timeout value (units of 10ms, 100=1s) if automatic parameter
// update request is enabled
#define DEFAULT_DESIRED_CONN_TIMEOUT          300

// Whether to enable automatic parameter update request when a connection is
// formed
#define DEFAULT_ENABLE_UPDATE_REQUEST         GAPROLE_LINK_PARAM_UPDATE_WAIT_REMOTE_PARAMS

// Connection Pause Peripheral time value (in seconds)
#define DEFAULT_CONN_PAUSE_PERIPHERAL         6

// How often to perform periodic event (in msec)
#define OTA_PERIODIC_EVT_PERIOD               5000

// The size of an OAD packet.
#define OAD_PACKET_SIZE                       ((OAD_BLOCK_SIZE) + 2)

// Task configuration
#define OTA_TASK_PRIORITY                     1

#ifndef OTA_TASK_STACK_SIZE
#define OTA_TASK_STACK_SIZE                   644
#endif

#define OAD_STATE_CHANGE_EVT                  0x0001

// Internal Events for RTOS application
#define OAD_ICALL_EVT                         ICALL_MSG_EVENT_ID // Event_Id_31
#define OAD_QUEUE_EVT                         UTIL_QUEUE_EVENT_ID // Event_Id_30
#define OAD_DL_COMPLETE_EVT                   Event_Id_00

#define OAD_ALL_EVENTS                        (OAD_ICALL_EVT | OAD_QUEUE_EVT | OAD_DL_COMPLETE_EVT)

/*********************************************************************
 * TYPEDEFS
 */

// App event passed from profiles.
typedef struct
{
  appEvtHdr_t hdr; // Event header
} otaEvt_t;

/*********************************************************************
 * GLOBAL VARIABLES
 */

// Display Interface
Display_Handle dispHandle = NULL;

/*********************************************************************
 * LOCAL VARIABLES
 */

// Entity ID globally used to check for source and/or destination of messages
static ICall_EntityID selfEntity;

// Event globally used to post local events and pend on system and
// local events.
static ICall_SyncHandle syncEvent;

// Queue object used for app messages
static Queue_Struct appMsg;
static Queue_Handle appMsgQueue;

// OAD Queue
static Queue_Struct oadQ;
static Queue_Handle hOadQ;

// Task configuration
Task_Struct otaTask;
Char otaTaskStack[OTA_TASK_STACK_SIZE];

// Profile state and parameters
//static gaprole_States_t gapProfileState = GAPROLE_INIT;

// GAP - SCAN RSP data (max size = 31 bytes)
static uint8_t scanRspData[] =
{
  // complete name
  0x13,   // length of this data
  GAP_ADTYPE_LOCAL_NAME_COMPLETE,
  'O',
  'A',
  'D',
  ' ',
  'T',
  'a',
  'r',
  'g',
  'e',
  't',
  ' ',
  'I',
  'm',
  'a',
  'g',
  'e',
  ' ',
  'A',

  // connection interval range
  0x05,   // length of this data
  GAP_ADTYPE_SLAVE_CONN_INTERVAL_RANGE,
  LO_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),   // 100ms
  HI_UINT16(DEFAULT_DESIRED_MIN_CONN_INTERVAL),
  LO_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),   // 1s
  HI_UINT16(DEFAULT_DESIRED_MAX_CONN_INTERVAL),

  // Tx power level
  0x02,   // length of this data
  GAP_ADTYPE_POWER_LEVEL,
  0       // 0dBm
};

// GAP - Advertisement data (max size = 31 bytes, though this is
// best kept short to conserve power while advertisting)
static uint8_t advertData[] =
{
  // Flags; this sets the device to use limited discoverable
  // mode (advertises for 30 seconds at a time) instead of general
  // discoverable mode (advertises indefinitely)
  0x02,   // length of this data
  GAP_ADTYPE_FLAGS,
  DEFAULT_DISCOVERABLE_MODE | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED,

  // service UUID, to notify central devices what services are included
  // in this peripheral
  0x03,   // length of this data
  GAP_ADTYPE_16BIT_MORE,      // some of the UUID's, but not all
  LO_UINT16(OAD_SERVICE_UUID),
  HI_UINT16(OAD_SERVICE_UUID)
};

// GAP GATT Attributes
static uint8_t attDeviceName[GAP_DEVICE_NAME_LEN] = "OAD Target";

/*********************************************************************
 * LOCAL FUNCTIONS
 */

static void OadTargetApp_init( void );
static void OadTargetApp_taskFxn(UArg a0, UArg a1);

static void OadTargetApp_processStackMsg(ICall_Hdr *pMsg);
static void OadTargetApp_processGATTMsg(gattMsgEvent_t *pMsg);
static void OadTargetApp_processAppMsg(otaEvt_t *pMsg);
static void OadTargetApp_processStateChangeEvt(gaprole_States_t newState);
static void OadTargetApp_stateChangeCB(gaprole_States_t newState);
static void OadTargetApp_enqueueMsg(uint8_t event, uint8_t state);

void OadTargetApp_processOadWriteCB(uint8_t event, uint16_t connHandle,
                                           uint8_t *pData);

/*********************************************************************
 * PROFILE CALLBACKS
 */

// GAP Role Callbacks
static gapRolesCBs_t OadTargetApp_gapRoleCBs =
{
  OadTargetApp_stateChangeCB     // Profile State Change Callbacks
};

// OAD Target Profile
static oadTargetCBs_t OadTargetApp_oadCBs =
{
  OadTargetApp_processOadWriteCB // Write Callback.
};

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

/*********************************************************************
 * @fn      OadTargetApp_createTask
 *
 * @brief   Task creation function for the Simple BLE Peripheral.
 *
 * @param   None.
 *
 * @return  None.
 */
void OadTargetApp_createTask(void)
{
  Task_Params taskParams;

  // Configure task
  Task_Params_init(&taskParams);
  taskParams.stack = otaTaskStack;
  taskParams.stackSize = OTA_TASK_STACK_SIZE;
  taskParams.priority = OTA_TASK_PRIORITY;

  Task_construct(&otaTask, OadTargetApp_taskFxn, &taskParams, NULL);
}

/*********************************************************************
 * @fn      OadTargetApp_init
 *
 * @brief   Called during initialization and contains application
 *          specific initialization (ie. hardware initialization/setup,
 *          table initialization, power up notification, etc), and
 *          profile initialization/setup.
 *
 * @param   None.
 *
 * @return  None.
 */
uint32_t Mode;
static void OadTargetApp_init(void)
{
	// ******************************************************************
  // N0 STACK API CALLS CAN OCCUR BEFORE THIS CALL TO ICall_registerApp
  // ******************************************************************
  // Register the current thread as an ICall dispatcher application
  // so that the application can send and receive messages.
  ICall_registerApp(&selfEntity, &syncEvent);
  
#ifdef USE_RCOSC 
  RCOSC_enableCalibration(); 
#endif // USE_RCOSC

  Mode = *(volatile uint32_t *)0x50003FB4;
  
  // Hard code the BD Address till CC2650 board gets its own IEEE address
  //uint8 bdAddress[B_ADDR_LEN] = { 0xAD, 0xD0, 0x0A, 0xAD, 0xD0, 0x0A };
  //HCI_EXT_SetBDADDRCmd(bdAddress);

  // Set device's Sleep Clock Accuracy
  //HCI_EXT_SetSCACmd(40);

  // Create an RTOS queue for message from profile to be sent to app.
  appMsgQueue = Util_constructQueue(&appMsg);

  // Setup the GAP
  GAP_SetParamValue(TGAP_CONN_PAUSE_PERIPHERAL, DEFAULT_CONN_PAUSE_PERIPHERAL);

  // Setup the GAP Peripheral Role Profile
  {
    // For all hardware platforms, device starts advertising upon initialization
    uint8_t initialAdvertEnable = TRUE;

    // By setting this to zero, the device will go into the waiting state after
    // being discoverable for 30.72 second, and will not being advertising again
    // until the enabler is set back to TRUE
    uint16_t advertOffTime = 0;

    uint8_t enableUpdateRequest = DEFAULT_ENABLE_UPDATE_REQUEST;
    uint16_t desiredMinInterval = DEFAULT_DESIRED_MIN_CONN_INTERVAL;
    uint16_t desiredMaxInterval = DEFAULT_DESIRED_MAX_CONN_INTERVAL;
    uint16_t desiredSlaveLatency = DEFAULT_DESIRED_SLAVE_LATENCY;
    uint16_t desiredConnTimeout = DEFAULT_DESIRED_CONN_TIMEOUT;

    // Set the GAP Role Parameters
    GAPRole_SetParameter(GAPROLE_ADVERT_ENABLED, sizeof(uint8_t),
                         &initialAdvertEnable);
    GAPRole_SetParameter(GAPROLE_ADVERT_OFF_TIME, sizeof(uint16_t),
                         &advertOffTime);

    GAPRole_SetParameter(GAPROLE_SCAN_RSP_DATA, sizeof(scanRspData),
                         scanRspData);
    GAPRole_SetParameter(GAPROLE_ADVERT_DATA, sizeof(advertData), advertData);

    GAPRole_SetParameter(GAPROLE_PARAM_UPDATE_ENABLE, sizeof(uint8_t),
                         &enableUpdateRequest);
    GAPRole_SetParameter(GAPROLE_MIN_CONN_INTERVAL, sizeof(uint16_t),
                         &desiredMinInterval);
    GAPRole_SetParameter(GAPROLE_MAX_CONN_INTERVAL, sizeof(uint16_t),
                         &desiredMaxInterval);
    GAPRole_SetParameter(GAPROLE_SLAVE_LATENCY, sizeof(uint16_t),
                         &desiredSlaveLatency);
    GAPRole_SetParameter(GAPROLE_TIMEOUT_MULTIPLIER, sizeof(uint16_t),
                         &desiredConnTimeout);
  }

  // Set the GAP Characteristics
  GGS_SetParameter(GGS_DEVICE_NAME_ATT, GAP_DEVICE_NAME_LEN, attDeviceName);

  // Set advertising interval
  {
    uint16_t advInt = DEFAULT_ADVERTISING_INTERVAL;

    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MIN, advInt);
    GAP_SetParamValue(TGAP_LIM_DISC_ADV_INT_MAX, advInt);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MIN, advInt);
    GAP_SetParamValue(TGAP_GEN_DISC_ADV_INT_MAX, advInt);
  }

   // Initialize GATT attributes
  GGS_AddService(GATT_ALL_SERVICES);           // GAP
  GATTServApp_AddService(GATT_ALL_SERVICES);   // GATT attributes

  VOID OAD_addService();                       // OAD Profile
  OAD_register((oadTargetCBs_t *)&OadTargetApp_oadCBs);

  hOadQ = Util_constructQueue(&oadQ);

  // Start the Device
  VOID GAPRole_StartDevice(&OadTargetApp_gapRoleCBs);

  // Register with GAP for HCI/Host messages
  GAP_RegisterForMsgs(selfEntity);
  
#define APP_TX_PDU_SIZE 27
#define APP_RX_PDU_SIZE 27
#define APP_TX_TIME 328
#define APP_RX_TIME 328

//This API is documented in hci.h
  HCI_EXT_SetMaxDataLenCmd(APP_TX_PDU_SIZE, APP_TX_TIME, APP_RX_PDU_SIZE, APP_RX_TIME);
  
#if !defined (USE_LL_CONN_PARAM_UPDATE)
  // Get the currently set local supported LE features
  // The HCI will generate an HCI event that will get received in the main
  // loop
  HCI_LE_ReadLocalSupportedFeaturesCmd();
#endif // !defined (USE_LL_CONN_PARAM_UPDATE)
  
  
}

/*********************************************************************
 * @fn      OadTargetApp_taskFxn
 *
 * @brief   Application task entry point for the Simple BLE Peripheral.
 *
 * @param   a0, a1 - not used.
 *
 * @return  None.
 */
static void OadTargetApp_taskFxn(UArg a0, UArg a1)
{
  // Initialize application
  OadTargetApp_init();

  // Application main loop
  for (;;)
  {
    uint32_t events;

    events = Event_pend(syncEvent, Event_Id_NONE, OAD_ALL_EVENTS,
                        ICALL_TIMEOUT_FOREVER);

    if (events)
    {
      ICall_EntityID dest;
      ICall_ServiceEnum src;
      ICall_HciExtEvt *pMsg = NULL;

      if (ICall_fetchServiceMsg(&src, &dest,
                                (void **)&pMsg) == ICALL_ERRNO_SUCCESS)
      {
        if ((src == ICALL_SERVICE_CLASS_BLE) && (dest == selfEntity))
        {
          ICall_Stack_Event *pEvt = (ICall_Stack_Event *)pMsg;

          // Check for BLE stack events first
          if (pEvt->signature == 0xffff)
          {
            if (pEvt->event_flag & OAD_DL_COMPLETE_EVT)
            {
              // Re-start device
              OADTarget_systemReset();
            }
          }
          else
          {
            // Process inter-task message
            OadTargetApp_processStackMsg((ICall_Hdr *)pMsg);
          }
        }

        if (pMsg)
        {
          ICall_freeMsg(pMsg);
        }
      }

      // If RTOS queue is not empty, process app message.
      if (events & OAD_QUEUE_EVT)
      {
        while (!Queue_empty(appMsgQueue))
        {
          otaEvt_t *pMsg = (otaEvt_t *)Util_dequeueMsg(appMsgQueue);
          if (pMsg)
          {
            // Process message.
            OadTargetApp_processAppMsg(pMsg);

            // Free the space from the message.
            ICall_free(pMsg);
          }
        }

        while (!Queue_empty(hOadQ))
        {

          oadTargetWrite_t *oadWriteEvt = Queue_get(hOadQ);

          // Identify new image.
          if (oadWriteEvt->event == OAD_WRITE_IDENTIFY_REQ)
          {
            OAD_imgIdentifyWrite(oadWriteEvt->connHandle, oadWriteEvt->pData);
          }
          // Write a next block request.
          else if (oadWriteEvt->event == OAD_WRITE_BLOCK_REQ)
          {
            OAD_imgBlockWrite(oadWriteEvt->connHandle, oadWriteEvt->pData);
          }
          else if (oadWriteEvt->event == OAD_IMAGE_COMPLETE)
          {
            HCI_EXT_ConnEventNoticeCmd(oadWriteEvt->connHandle, selfEntity,
                                   OAD_DL_COMPLETE_EVT);
          }

          // Free buffer.
          ICall_free(oadWriteEvt);
        }
      }
    }
  }
}

/*********************************************************************
 * @fn      OadTargetApp_processStackMsg
 *
 * @brief   Process an incoming stack message.
 *
 * @param   pMsg - message to process
 *
 * @return  None.
 */
static void OadTargetApp_processStackMsg(ICall_Hdr *pMsg)
{
  switch (pMsg->event)
  {
    case GATT_MSG_EVENT:
      // Process GATT message
      OadTargetApp_processGATTMsg((gattMsgEvent_t *)pMsg);
      break;

    case HCI_GAP_EVENT_EVENT:
      {

        // Process HCI message
        switch(pMsg->status)
        {
          case HCI_COMMAND_COMPLETE_EVENT_CODE:
            // Process HCI Command Complete Event
            {

#if !defined (USE_LL_CONN_PARAM_UPDATE)
              // This code will disable the use of the LL_CONNECTION_PARAM_REQ
              // control procedure (for connection parameter updates, the 
              // L2CAP Connection Parameter Update procedure will be used 
              // instead). To re-enable the LL_CONNECTION_PARAM_REQ control
              // procedures, define the symbol USE_LL_CONN_PARAM_UPDATE                
              
              // Parse Command Complete Event for opcode and status
              hciEvt_CmdComplete_t* command_complete = (hciEvt_CmdComplete_t*) pMsg;
              uint8_t   pktStatus = command_complete->pReturnParam[0]; 
              
              //find which command this command complete is for
              switch (command_complete->cmdOpcode)
              {
                case HCI_LE_READ_LOCAL_SUPPORTED_FEATURES:
                  {
                    if (pktStatus == SUCCESS)
                    {
                      uint8_t featSet[8];
                      
                      // get current feature set from received event (bits 1-9 of
                      // the returned data
                      memcpy( featSet, &command_complete->pReturnParam[1], 8 );                          
                      
                      // Clear bit 1 of byte 0 of feature set to disable LL
                      // Connection Parameter Updates
                      CLR_FEATURE_FLAG( featSet[0], LL_FEATURE_CONN_PARAMS_REQ );
                        
                      // Update controller with modified features
                      HCI_EXT_SetLocalSupportedFeaturesCmd( featSet );
                    }
                  }
                  break;
                
                default:
                  //do nothing
                  break;
              }   
#endif // !defined (USE_LL_CONN_PARAM_UPDATE)
              
            }
            break;

          default:
            break;
        }
      }
      break;
      
    default:
      // do nothing
      break;
  }
}

/*********************************************************************
 * @fn      OadTargetApp_processGATTMsg
 *
 * @brief   Process GATT messages
 *
 * @return  None.
 */
static void OadTargetApp_processGATTMsg(gattMsgEvent_t *pMsg)
{
  GATT_bm_free(&pMsg->msg, pMsg->method);
}

/*********************************************************************
 * @fn      OadTargetApp_processAppMsg
 *
 * @brief   Process an incoming callback from a profile.
 *
 * @param   pMsg - message to process
 *
 * @return  None.
 */
static void OadTargetApp_processAppMsg(otaEvt_t *pMsg)
{
  switch (pMsg->hdr.event)
  {
    case OAD_STATE_CHANGE_EVT:
      OadTargetApp_processStateChangeEvt((gaprole_States_t)pMsg->hdr.state);
      break;

    default:
      // Do nothing.
      break;
  }
}

/*********************************************************************
 * @fn      OadTargetApp_stateChangeCB
 *
 * @brief   Callback from GAP Role indicating a role state change.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void OadTargetApp_stateChangeCB(gaprole_States_t newState)
{
  OadTargetApp_enqueueMsg(OAD_STATE_CHANGE_EVT, newState);
}

/*********************************************************************
 * @fn      OadTargetApp_processStateChangeEvt
 *
 * @brief   Process a pending GAP Role state change event.
 *
 * @param   newState - new state
 *
 * @return  None.
 */
static void OadTargetApp_processStateChangeEvt(gaprole_States_t newState)
{
  switch ( newState )
  {
    case GAPROLE_STARTED:
      break;

    case GAPROLE_ADVERTISING:
      break;

    case GAPROLE_CONNECTED:
      break;

    case GAPROLE_WAITING:
      break;

    case GAPROLE_WAITING_AFTER_TIMEOUT:
      break;

    case GAPROLE_ERROR:
      break;

    default:
      break;
  }
}

/*********************************************************************
 * @fn      OadTargetApp_processOadWriteCB
 *
 * @brief   Process a write request to the OAD profile.
 *
 * @param   event      - event type:
 *                       OAD_WRITE_IDENTIFY_REQ
 *                       OAD_WRITE_BLOCK_REQ
 * @param   connHandle - the connection Handle this request is from.
 * @param   pData      - pointer to data for processing and/or storing.
 *
 * @return  None.
 */
void OadTargetApp_processOadWriteCB(uint8_t event, uint16_t connHandle,
                                           uint8_t *pData)
{
  oadTargetWrite_t *oadWriteEvt = ICall_malloc( sizeof(oadTargetWrite_t) + \
                                             sizeof(uint8_t) * OAD_PACKET_SIZE);

  if ( oadWriteEvt != NULL )
  {
    oadWriteEvt->event = event;
    oadWriteEvt->connHandle = connHandle;

    oadWriteEvt->pData = (uint8_t *)(&oadWriteEvt->pData + 1);
    memcpy(oadWriteEvt->pData, pData, OAD_PACKET_SIZE);

    Queue_put(hOadQ, (Queue_Elem *)oadWriteEvt);

    Event_post(syncEvent, OAD_QUEUE_EVT);
  }
  else
  {
    // Fail silently.
  }
}

/*********************************************************************
 * @fn      OadTargetApp_enqueueMsg
 *
 * @brief   Creates a message and puts the message in RTOS queue.
 *
 * @param   event  - message event.
 * @param   state - message state.
 *
 * @return  None.
 */
static void OadTargetApp_enqueueMsg(uint8_t event, uint8_t state)
{
  otaEvt_t *pMsg;

  // Create dynamic pointer to message.
  if ((pMsg = ICall_malloc(sizeof(otaEvt_t))))
  {
    pMsg->hdr.event = event;
    pMsg->hdr.state = state;

    // Enqueue the message.
    Util_enqueueMsg(appMsgQueue, syncEvent, (uint8*)pMsg);
  }
}

/*********************************************************************
*********************************************************************/
