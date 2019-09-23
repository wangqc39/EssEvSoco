#include "ble_nus.h"
#include "BleDataLayer.h"
#include "app_error.h"
#include "nrf_log.h"
#include "ActionTick.h"

#define BLE_RX_NODE_CNT         10

struct BleRxNode
{
    uint8_t Buff[250];
    uint16_t cnt;
};

struct BleRxInfo
{
    uint8_t tail, head;
    struct BleRxNode Node[BLE_RX_NODE_CNT];
};

struct BleRxInfo BleRx;


BLE_NUS_DEF(m_nus, NRF_SDH_BLE_TOTAL_LINK_COUNT);                                   /**< BLE NUS service instance. */

static void nus_data_handler(ble_nus_evt_t * p_evt)
{
    if (p_evt->type == BLE_NUS_EVT_RX_DATA)
    {
        NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);

        BleRx.Node[BleRx.tail].Buff[0] = 0x70;       

        memcpy(BleRx.Node[BleRx.tail].Buff + 1, p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
        BleRx.Node[BleRx.tail].cnt = p_evt->params.rx_data.length + 1;
        BleRx.tail++;
        if(BleRx.tail >= BLE_RX_NODE_CNT)
            BleRx.tail = 0;
        //WriteSoundBlockDataCmdHandler((uint8_t *)p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
    }
    else if(p_evt->type == BLE_NUS_EVT_RX_CMD)
    {
        NRF_LOG_DEBUG("Received data from BLE NUS. Writing data on UART.");
        NRF_LOG_HEXDUMP_DEBUG(p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
        //BleAnalysisMessageId((uint8_t *)p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
        memcpy(BleRx.Node[BleRx.tail].Buff, p_evt->params.rx_data.p_data, p_evt->params.rx_data.length);
        BleRx.Node[BleRx.tail].cnt = p_evt->params.rx_data.length;
        BleRx.tail++;
        if(BleRx.tail >= BLE_RX_NODE_CNT)
            BleRx.tail = 0;
    }

}




void BleProtocalServiceInit()
{
    ble_nus_init_t     nus_init;
    uint32_t err_code;
    nus_init.data_handler = nus_data_handler;

    err_code = ble_nus_init(&m_nus, &nus_init);
    APP_ERROR_CHECK(err_code);
}


extern uint16_t m_conn_handle;
void BleSendOneFrame(unsigned char *buff, uint16_t cnt)
{
    ble_nus_data_send(&m_nus, buff, &cnt, m_conn_handle);
}


void BleRxDataHandler()
{
    while(BleRx.head != BleRx.tail)
    {
        BleAnalysisMessageId(BleRx.Node[BleRx.head].Buff, BleRx.Node[BleRx.head].cnt);
        BleRx.head++;
        if(BleRx.head >= BLE_RX_NODE_CNT)
            BleRx.head = 0;
    }
}


#define MAX_CNT_BLE_REQUEST_CMD_NODE    20
//所有的请求节点为2-3字节
//第一字节为命令字
//获取车辆信息会带一个车辆ID，所以是3字节
struct BleRequestCmdNodeInfo
{
    uint8_t cmd[3];
    uint8_t length;
};

struct BleRequsetCmdTableInfo
{
    uint32_t LastHandleTime;
    struct BleRequestCmdNodeInfo node[MAX_CNT_BLE_REQUEST_CMD_NODE];
    uint8_t head, tail;
};

struct BleRequsetCmdTableInfo BleRequestCmdTable;


/** 
 * [AddNodeToBleRequestCmdTable description]增加一个BLE请求消息的节点，每个请求消息的节点会对应一次BLE的数据返回
 * @Author   tin39
 * @DateTime 2019年7月25日T10:01:50+0800
 * @param    data                     [description]
 */
void AddNodeToBleRequestCmdTable(uint8_t *data, uint8_t length)
{
    struct BleRequestCmdNodeInfo *ThisNode = &BleRequestCmdTable.node[BleRequestCmdTable.tail];
    memcpy(ThisNode->cmd, data, length);
    ThisNode->length = length;
    BleRequestCmdTable.tail++;
    if(BleRequestCmdTable.tail >= MAX_CNT_BLE_REQUEST_CMD_NODE)
        BleRequestCmdTable.tail = 0;
}

/** 
 * [BleRequestCmdTableHandler description]对BLE request table中的请求信息进行处理，每次处理1个请求，每次处理之间有处理间隔
 * @Author   tin39
 * @DateTime 2019年7月25日T10:20:51+0800
 * @param                             [description]
 */
void BleRequestCmdTableHandler()
{
    if(BleRequestCmdTable.head == BleRequestCmdTable.tail)
        return;

    if(GetSystemTime() <   BleRequestCmdTable.LastHandleTime + MS(10))
        return;

    struct BleRequestCmdNodeInfo *ThisNode = &BleRequestCmdTable.node[BleRequestCmdTable.head];
    BleAnalysisMessageId(ThisNode->cmd, ThisNode->length);

    BleRequestCmdTable.head++;
    if(BleRequestCmdTable.head >= MAX_CNT_BLE_REQUEST_CMD_NODE)
        BleRequestCmdTable.head = 0;
    BleRequestCmdTable.LastHandleTime =  GetSystemTime();
}




