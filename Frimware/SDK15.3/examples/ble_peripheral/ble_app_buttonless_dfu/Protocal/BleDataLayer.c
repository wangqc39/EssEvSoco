#include "common.h"
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include "SystemInfo.h"
#include "BleTransportLayer.h"
#include "SystemConfig.h"
#include "MixerConfig.h"
#include "Mixer.h"
#include "DownLoader.h"
#include "BleError.h"
#include "Password.h"
#include "nrf_delay.h"
#include "BleService.h"








#define CMD_SYSTEM_INFO_REQUEST			0x10
#define CMD_SYSTEM_INFO_REQUEST_RES		CMD_SYSTEM_INFO_REQUEST + 0x80

#define CMD_CHANGE_PROGRAM				0x5A
#define CMD_CHANGE_PROGRAM_RES			CMD_CHANGE_PROGRAM + 0x80

#define CMD_SYSTEM_PARAM_REQUEST			0x20
#define CMD_SYSTEM_PARAM_REQUEST_RES		CMD_SYSTEM_PARAM_REQUEST + 0x80

#define CMD_SYSTEM_PARAM_SET				0x21
#define CMD_SYSTEM_PARAM_SET_RES			CMD_SYSTEM_PARAM_SET + 0x80

#define CMD_SYSTEM_PARAM_STORE			0x22
#define CMD_SYSTEM_PARAM_STORE_RES		CMD_SYSTEM_PARAM_STORE + 0x80

#define CMD_VEHICLE_PARAM_REQUEST			0x30
#define CMD_VEHICLE_PARAM_REQUEST_RES		CMD_VEHICLE_PARAM_REQUEST + 0x80

#define CMD_VEHICLE_PARAM_SET				0x31
#define CMD_VEHICLE_PARAM_SET_RES			CMD_VEHICLE_PARAM_SET + 0x80

#define CMD_VEHICLE_PARAM_STORE			0x32
#define CMD_VEHICLE_PARAM_STORE_RES		CMD_VEHICLE_PARAM_STORE + 0x80

#define CMD_FORMAT_FS						0x40
#define CMD_FORMAT_FS_RES					CMD_FORMAT_FS + 0x80

#define CMD_READ_SOUND_GUID				0x41
#define CMD_READ_SOUND_GUID_RES			CMD_READ_SOUND_GUID + 0x80

#define CMD_READ_SOUND_NAME				0x42
#define CMD_READ_SOUND_NAME_RES			CMD_READ_SOUND_NAME + 0x80

#define CMD_WRITE_SOUND_START				0x43
#define CMD_WRITE_SOUND_START_RES			CMD_WRITE_SOUND_START + 0x80

#define CMD_GET_NEXT_SOUND_BLOCK_INDEX	0x44
#define CMD_GET_NEXT_SOUND_BLOCK_INDEX_RES	CMD_GET_NEXT_SOUND_BLOCK_INDEX + 0x80

#define CMD_WRITE_SOUND_BLOCK_DATA		0x70

#define CMD_CHECK_SOUND_BLOCK_DATA		0x45
#define CMD_CHECK_SOUND_BLOCK_DATA_RES	CMD_CHECK_SOUND_BLOCK_DATA + 0x80

#define CMD_WIRTE_SOUND_FINISH				0x46
#define CMD_WIRTE_SOUND_FINISH_RES		CMD_WIRTE_SOUND_FINISH + 0x80

#define CMD_DELETE_SOUND					0x48
#define CMD_DELETE_SOUND_RES				CMD_DELETE_SOUND + 0x80

#define CMD_SELECT_VEHICLE					0x49
#define CMD_SELECT_VEHICLE_RES				CMD_SELECT_VEHICLE + 0x80

#define CMD_SET_SOUND_DATA_PACKET_DATA_LENGTH       0x4A
#define CMD_SET_SOUND_DATA_PACKET_DATA_LENGTH_RES       CMD_SET_SOUND_DATA_PACKET_DATA_LENGTH + 0x80


#define CMD_GET_RANDOM                      0x60
#define CMD_GET_RANDOM_RES                  CMD_GET_RANDOM + 0x80
    
#define CMD_AUTHORIZATION                   0x61
#define CMD_AUTHORIZATION_RES               CMD_AUTHORIZATION + 0x80

#define CMD_SET_PASSWORD                    0x62
#define CMD_SET_PASSWORD_RES                CMD_SET_PASSWORD + 0x80

#define CMD_RESET_PASSWORD                  0x63
#define CMD_RESET_PASSWORD_RES              CMD_RESET_PASSWORD + 0x80

#define CMD_BLE_DISCONNECT                  0x64
#define CMD_BLE_DISCONNECT_RES              CMD_BLE_DISCONNECT + 0x80



static uint8_t MultiRequestBuff[100];
int32_t SystemInfoRequestCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    uint32_t i;
    uint8_t SendBuff[20];
    uint8_t index;
    int32_t DataLength;
    if(length == 0 || length > 19)
    {
        ErrorCode = ERROR_SYSTEM_INFO_REQUEST_LENGTH + length;
        BleSendErrorPacket(CMD_SYSTEM_INFO_REQUEST_RES, ErrorCode);
        return -1;
    }

    if(length == 1)
    {
        //一次请求一个项目，直接返回结果
        //通过request table来单次请求一个项目，也会进入本流程
        index = buff[0];
        DataLength = GetOneSystemInfo(SendBuff + 2, index);
        if(DataLength > 0)
        {
            //获取到SystemInfo,发送给APP
            SendBuff[0] = CMD_SYSTEM_INFO_REQUEST_RES;
            SendBuff[1] = index;
            BleSendOneFrame(SendBuff, 2 + DataLength);
        }
        else
        {
            //Index不在范围内
            ErrorCode = ERROR_SYSTEM_INFO_REQUEST_INDEX + index;
            BleSendErrorPacket(CMD_SYSTEM_INFO_REQUEST_RES, ErrorCode);
        }
    }
    else
    {
        //一次请求多个项目，则添加到table中
        uint8_t cmd[2];
        cmd[0] = CMD_SYSTEM_INFO_REQUEST;
        for(i = 0; i < length; i++)
        {
            cmd[1] = buff[i];
            AddNodeToBleRequestCmdTable(cmd, 2);
        }
    }


//    for(i = 0; i < length; i++)
//    {
//        index = buff[i];
//        DataLength = GetOneSystemInfo(SendBuff + 2, index);
//        if(DataLength > 0)
//        {
//            //获取到SystemInfo,发送给APP
//            SendBuff[0] = CMD_SYSTEM_INFO_REQUEST_RES;
//            SendBuff[1] = index;
//            BleSendOneFrame(SendBuff, 2 + DataLength);
//            mDelay(10);
//        }
//        else
//        {
//            //Index不在范围内
//            ErrorCode = ERROR_SYSTEM_INFO_REQUEST_INDEX + index;
//            BleSendErrorPacket(CMD_SYSTEM_INFO_REQUEST_RES, ErrorCode);
//        }
//    }
    //将所有消息存到request table中，分时进行处理
    

    return 0;
}


int32_t ChangeProgeramCmdHandler()
{
    uint8_t SendBuff[20];
    uint32_t ErrorCode;
    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }
    
    SendBuff[0] = CMD_CHANGE_PROGRAM_RES;
    BleSendOneFrame(SendBuff, 1);

    mDelay(10);

    //SetBootData(BOOT_BOOTLOADER);
    //IwdgInit();
    
    while(1);
    
Error:
    BleSendErrorPacket(CMD_CHANGE_PROGRAM_RES, ErrorCode);
    return ErrorCode;

}


int32_t SystemParamRequestCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    uint32_t i;
    uint8_t SendBuff[20];
    uint8_t index;
    int32_t DataLength;

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }

    if(length == 0 || length > 19)
    {
        ErrorCode = ERROR_SYSTEM_PARAM_REQUEST_LENGTH + length;
        goto Error;
    }

    if(length == 1)
    {
        //一次请求一个项目，直接返回结果
        //通过request table来单次请求一个项目，也会进入本流程
        index = buff[0];
        DataLength = GetOneSystemParam(SendBuff + 2, index);
        if(DataLength > 0)
        {
            //获取到SystemInfo,发送给APP
            SendBuff[0] = CMD_SYSTEM_PARAM_REQUEST_RES;
            SendBuff[1] = index;
            BleSendOneFrame(SendBuff, 2 + DataLength);
        }
        else
        {
            //Index不在范围内
            ErrorCode = CMD_SYSTEM_PARAM_REQUEST_RES + index;
            goto Error;
        }
    }
    else
    {
        //一次请求多个项目，则添加到table中
        uint8_t cmd[2];
        cmd[0] = CMD_SYSTEM_PARAM_REQUEST;
        for(i = 0; i < length; i++)
        {
            cmd[1] = buff[i];
            AddNodeToBleRequestCmdTable(cmd, 2);
        }
    }
    return 0;
Error:
    BleSendErrorPacket(CMD_SYSTEM_PARAM_REQUEST_RES, ErrorCode);
    return ErrorCode;

    
}

int32_t SystemParamSetCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    uint8_t SendBuff[20];
    uint8_t index;
    int32_t DataLength;

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }
    
    if(length < 2 || length > 19)
    {
        ErrorCode = ERROR_SYSTEM_PARAM_SET_LENGTH + length;
        goto Error;
    }

    index = buff[0];
    DataLength = SetOneSystemParam(buff + 1, SendBuff + 2, length - 1, index);
    if(DataLength > 0)
    {
        SendBuff[0] = CMD_SYSTEM_PARAM_SET_RES;
        SendBuff[1] = index;
        BleSendOneFrame(SendBuff, 2 + DataLength);
    }
    else
    {
        //Index不在范围内
        ErrorCode = ERROR_SYSTEM_PARAM_SET_INDEX + index;
        goto Error;
    }

    return 0;

Error:
    BleSendErrorPacket(CMD_SYSTEM_PARAM_SET_RES, ErrorCode);
    return ErrorCode;
}

int32_t SystemParamStoreCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    uint8_t SendBuff[20];

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }

    
    if(length != 0)
    {
        ErrorCode = ERROR_SYSTEM_PARAM_STORE_LENGTH;
        BleSendErrorPacket(CMD_SYSTEM_PARAM_STORE_RES, ErrorCode);
        return -1;
    }

    WriteSystemParamTable();
    SendBuff[0] = CMD_SYSTEM_PARAM_STORE_RES;
    SendBuff[1] = 0;
    BleSendOneFrame(SendBuff, 2);

    
    return 0;

Error:
    BleSendErrorPacket(CMD_SYSTEM_PARAM_STORE_RES, ErrorCode);
    return ErrorCode;    
}


int32_t VehicleParamRequestCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    uint32_t i;
    uint8_t SendBuff[20];
    uint8_t index;
    uint8_t VehicleIndex;
    int32_t DataLength;

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }

    
    if(length < 2 || length > 19)
    {
        ErrorCode = ERROR_VEHICLE_PARAM_REQUEST_LENGTH + length;
        goto Error;
    }


    //Todo:无法获取除当前车辆外的车辆配置
    VehicleIndex = buff[0];
    if(VehicleIndex != mixer.SoundIndex)
    {
        ErrorCode = ERROR_VEHICLE_PARAM_REQUEST_VEHICLE_INDEX + VehicleIndex;
        goto Error;
    }

    if(length == 2)
    {
        index = buff[1];
        DataLength = GetOneVehicleParam(SendBuff + 3, index);
        if(DataLength > 0)
        {
            //获取到VehicleParam,发送给APP
            SendBuff[0] = CMD_VEHICLE_PARAM_REQUEST_RES;
            SendBuff[1] = VehicleIndex;
            SendBuff[2] = index;
            BleSendOneFrame(SendBuff, 3 + DataLength);
        }
        else
        {
            //Index不在范围内
            ErrorCode = ERROR_VEHICLE_PARAM_REQUEST_INDEX + index;
            goto Error;
        }
    }
    else
    {
        //一次请求多个项目，则添加到table中
        uint8_t cmd[3];
        cmd[0] = CMD_VEHICLE_PARAM_REQUEST;
        cmd[1] = VehicleIndex;
        for(i = 0; i < length - 1; i++)
        {
            cmd[2] = buff[i + 1];
            AddNodeToBleRequestCmdTable(cmd, 3);
        }
    }

    return 0;

Error:
    BleSendErrorPacket(CMD_VEHICLE_PARAM_REQUEST_RES, ErrorCode);
    return ErrorCode;    

}


int32_t VehicleParamSetCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    uint8_t SendBuff[20];
    uint8_t index;
    uint8_t VehicleIndex;
    int32_t DataLength;

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }

    
    if(length < 3 || length > 19)
    {
        ErrorCode = ERROR_VEHICLE_PARAM_SET_LENGTH + length;
        goto Error;
    }

    //Todo:无法获取除当前车辆外的车辆配置
    VehicleIndex = buff[0];
    if(VehicleIndex != mixer.SoundIndex)
    {
        ErrorCode = ERROR_VEHICLE_PARAM_SET_VEHICLE_INDEX + VehicleIndex;
        goto Error;
    }

    index = buff[1];
    DataLength = SetOneVehicleParam(buff + 2, SendBuff + 3, length - 2, index);
    if(DataLength > 0)
    {
        SendBuff[0] = CMD_VEHICLE_PARAM_SET_RES;
        SendBuff[1] = VehicleIndex;
        SendBuff[2] = index;
        BleSendOneFrame(SendBuff, 3 + DataLength);
    }
    else
    {
        //Index不在范围内
        ErrorCode = ERROR_VEHICLE_PARAM_SET_INDEX + index;
        goto Error;
    }

    return 0;

Error:
    BleSendErrorPacket(CMD_VEHICLE_PARAM_SET_RES, ErrorCode);
    return ErrorCode;    

}

int32_t VehicleParamStoreCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    uint8_t SendBuff[20];
    uint8_t VehicleIndex;

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }
    
    if(length != 1)
    {
        ErrorCode = ERROR_VEHICLE_PARAM_STORE_LENGTH;
        goto Error;
    }

    //Todo:无法获取除当前车辆外的车辆配置
    VehicleIndex = buff[0];
    if(VehicleIndex != mixer.SoundIndex)
    {
        ErrorCode = ERROR_VEHICLE_PARAM_STORE_VEHICLE_INDEX + VehicleIndex;
        goto Error;
    }

    WriteVehicleParamTable(VehicleIndex);
    SendBuff[0] = CMD_VEHICLE_PARAM_STORE_RES;
    SendBuff[1] = VehicleIndex;
    SendBuff[2] = 0;
    BleSendOneFrame(SendBuff, 3);

    
    return 0;
Error:
    BleSendErrorPacket(CMD_VEHICLE_PARAM_STORE_RES, ErrorCode);
    return ErrorCode;    

}

int32_t FormatFsCmdHandler()
{
    unsigned char SendBuffer[20];
    uint16_t ret;
    uint32_t ErrorCode;

    if(IsPasswordPassed() == false)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }
    
    ret = FormatFs();
    GetFsInfo();

    if(ret != 0)
    {
        ErrorCode = ERROR_FORMAT_FS + ret; 
        goto Error;
    }

    
    SendBuffer[0] = CMD_FORMAT_FS_RES;
    SendBuffer[1] = 0;
    BleSendOneFrame(SendBuffer, 2);
    return 0;

Error:
    BleSendErrorPacket(CMD_FORMAT_FS_RES, ErrorCode);
    return ErrorCode;    

}

int32_t ReadSoundGuidCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    uint8_t VehicleIndex;
    uint8_t SoundType;
    uint8_t SendBuff[20];
    struct FileInfo *fp;
    unsigned char FileWholeFlag;

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }
    
    VehicleIndex = buff[0];
    if(VehicleIndex >= VEHICLE_CNT)
    {
        ErrorCode = ERROR_READ_SOUND_GUID_INDEX + VehicleIndex;
        goto Error;
    }
 
    SoundType = buff[1];
    if(SoundType > TYPE_ENGINE_SOUND)
    {
        ErrorCode = ERROR_READ_SOUND_GUID_SOUND_TYPE + SoundType;
        goto Error;
    }

    fp = &File[ENGINE_INDEX(VehicleIndex)];
    DataFlashReadData(fp->FileIndexAddr + OFFSET_FILE_WHOLE_FLAG, &FileWholeFlag, SIZE_ENABLE_FLAG);
    if(FileWholeFlag != 0)
    {
        //若声音无效，GUID返回0
        memset(SendBuff + 3, 0xFF, 16);
    }
    else
    {
        DataFlashReadData(fp->FileIndexAddr + OFFSET_FILE_GUID, SendBuff + 3, SIZE_GUID);
    }
    SendBuff[0] = CMD_READ_SOUND_GUID_RES;
    SendBuff[1] = VehicleIndex;
    SendBuff[2] = SoundType;
    BleSendOneFrame(SendBuff, 19);

    return 0;
    
Error:
    BleSendErrorPacket(CMD_READ_SOUND_GUID_RES, ErrorCode);
    return ErrorCode;
}

int32_t ReadSoundNameCmdHandler(uint8_t *buff, uint32_t length)
{
    uint8_t SendBuff[20];
    uint32_t ErrorCode;
    uint8_t VehicleIndex;
    uint8_t SoundType;
    struct FileInfo *fp;
    unsigned char FileWholeFlag;
    uint8_t SoundName[40];
    uint32_t NameRemainCnt;
    uint8_t SendCnt;
    uint8_t PacketIndex;
    uint8_t TotalPacketCnt;

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }
    
    VehicleIndex = buff[0];
    if(VehicleIndex >= VEHICLE_CNT)
    {
        ErrorCode = ERROR_READ_SOUND_NAME_INDEX + VehicleIndex;
        goto Error;
    }
 
    SoundType = buff[1];
    if(SoundType > TYPE_ENGINE_SOUND)
    {
        ErrorCode = ERROR_READ_SOUND_NAME_SOUND_TYPE + SoundType;
        goto Error;
    }

    fp = &File[ENGINE_INDEX(VehicleIndex)];
    DataFlashReadData(fp->FileIndexAddr + OFFSET_FILE_WHOLE_FLAG, &FileWholeFlag, SIZE_ENABLE_FLAG);
    if(FileWholeFlag != 0)
    {
        //若声音无效，SoundName字段为空
        SendBuff[0] = CMD_READ_SOUND_NAME_RES;
        SendBuff[1] = VehicleIndex;
        SendBuff[2] = SoundType;
        SendBuff[3] = 1;
        SendBuff[4] = 0;
        BleSendOneFrame(SendBuff, 5);
    }
    else
    {
        DataFlashReadData(fp->FileIndexAddr + OFFSET_FILE_NAME, SoundName, SIZE_SOUND_NAME);
        //获取Name的长度
        NameRemainCnt = sizeof(SoundName);
        if(NameRemainCnt > 40)
            NameRemainCnt = 40;

        //计算需要发送的数据包数量
        if(NameRemainCnt <= 15)
        {
            TotalPacketCnt = 1;
        }
        else if(NameRemainCnt <= 30)
        {
            TotalPacketCnt = 2;
        }
        else
        {
            TotalPacketCnt = 3;
        }

        PacketIndex = 0;
        SendBuff[0] = CMD_READ_SOUND_NAME_RES;
        SendBuff[1] = VehicleIndex;
        SendBuff[2] = SoundType;
        SendBuff[3] = TotalPacketCnt;
        while(NameRemainCnt > 0)
        {
            if(NameRemainCnt > 15)
            {
                SendCnt = 15;
            }
            else
            {
                SendCnt = NameRemainCnt;
            }
            SendBuff[4] = PacketIndex;
            memcpy(SendBuff + 5, SoundName + 15 * PacketIndex, SendCnt);
            BleSendOneFrame(SendBuff, 5 + SendCnt);

            PacketIndex++;
            NameRemainCnt -= SendCnt;
        }
    }
    return 0;
    
Error:
    BleSendErrorPacket(CMD_READ_SOUND_NAME_RES, ErrorCode);
    return ErrorCode;   
}


int32_t WriteSoundStartCmdHandler(uint8_t *buff, uint32_t length)
{
     uint32_t ErrorCode;
     int32_t ret;
     static uint8_t TotalPacketCnt;
     static uint8_t PacketIndex;
     static bool DataEmptyFlag = TRUE;;//true:未收到数据包，等待第一个数据包;false:已经收到了第一个数据包
     static uint8_t TotalDataCnt;
     uint8_t SendBuff[20];

     if(IsPasswordPassed() == FALSE)
     {
         ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
         goto Error;
     }

     if(DataEmptyFlag == TRUE)
     {
         //第一个数据包
         if(length != 19)
         {
             ErrorCode = ERROR_WRITE_SOUND_START_TOTAL_LENGTH | length;
             goto Error;
         }

         //检查数据包总数和Index值
         TotalPacketCnt = buff[0];
         PacketIndex = buff[1];
         if(TotalPacketCnt < 2 || TotalPacketCnt > 4)
         {
             ErrorCode = ERROR_WRITE_SOUND_START_TOTAL_PACKET_CNT | TotalPacketCnt;
             goto Error;
         }

         if(PacketIndex != 0)
         {
             ErrorCode = ERROR_WRITE_SOUND_START_FIRST_PACKET_INDEX | PacketIndex;
             goto Error;
         }

         //第一个数据包合法，进行数据缓存
         memcpy(MultiRequestBuff, buff + 2, 17);
         TotalDataCnt = 17;
         DataEmptyFlag = FALSE;
     }
     else
     {
         //后续的数据包

         //确认包总数是否一致
         if(TotalPacketCnt != buff[0])
         {
             ErrorCode = ERROR_WRITE_SOUND_START_TOTAL_PACKET_CNT_CONTINUE | buff[0];
             goto Error;
         }

         //确认PacketIndex递增是否正常
         if(PacketIndex + 1 != buff[1])
         {
             ErrorCode = ERROR_WRITE_SOUND_START_CONTINUE_PACKET_INDEX | buff[1];
             goto Error;
         }

         
         if(buff[1] + 1 != buff[0])
         {
             //不是最后一个数据包
             if(length != 19)
             {
                 //检查数据包内容不满
                 ErrorCode = ERROR_WRITE_SOUND_START_TOTAL_LENGTH_CONTINUE | length;
                 goto Error;
             }
         }

         //检查通过，缓存数据
         PacketIndex = buff[1];
         memcpy(MultiRequestBuff + PacketIndex * 17, buff + 2, length - 2);
         TotalDataCnt += length - 2;

         //检查是否是最后一个数据包
         if(PacketIndex + 1 == TotalPacketCnt)
         {
             ret = WriteSoundStart(MultiRequestBuff, TotalDataCnt);
             if(ret == 0)
             {
                 //音频烧写开始成功
                 SendBuff[0] = CMD_WRITE_SOUND_START_RES;
                 BleSendOneFrame(SendBuff, 1);
                 DataEmptyFlag = TRUE;
             }
             else
             {
                 ErrorCode = ERROR_WRITE_SOUND_START_WRITE_SOUND_START | (uint16_t)ret;
                 goto Error;
             }
         }

         
     }

     return 0;
Error:
    DataEmptyFlag = TRUE;
    BleSendErrorPacket(CMD_WRITE_SOUND_START_RES, ErrorCode);
    return ErrorCode;   
}

int32_t SetDataLengthCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    uint8_t SendBuff[2] = {CMD_SET_SOUND_DATA_PACKET_DATA_LENGTH_RES, 0};
    uint16_t DataLength;
    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }

    if(length != 2)
    {
        ErrorCode = ERROR_SET_DATA_LENGTH_LENGTH;
        goto Error;
    }

    
    memcpy(&DataLength, buff, 2);
    if(DataLength != 19 && DataLength != 243)
    {
        ErrorCode = ERROR_SET_DATA_LENGTH_CONTENT;
        goto Error;
    }

    SetDlPacketDataCnt(DataLength);

    
    BleSendOneFrame(SendBuff, 2);

    return 0;

Error:
    BleSendErrorPacket(CMD_SET_SOUND_DATA_PACKET_DATA_LENGTH_RES, ErrorCode);
    return ErrorCode;   
}


int32_t GetNextSoundBlockIndexCmdHandler(uint8_t *buff, uint32_t length)
{
    int32_t NextBlockIndex;
    uint8_t SendBuff[2];
    uint32_t ErrorCode;

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }
    
    NextBlockIndex = GetNextSoundBlockIndex();
    if(NextBlockIndex < 0)
    {
        ErrorCode = ERROR_GET_NEXT_SOUND_BLOCK_INDEX | (uint16_t)NextBlockIndex;
        goto Error;
    }

    
    SendBuff[0] = CMD_GET_NEXT_SOUND_BLOCK_INDEX_RES;
    if(NextBlockIndex == 10000)
    {
        //整个文件都接收完成
        BleSendOneFrame(SendBuff, 1);
    }
    else
    {
        //上报尚未接收数据块的index
        SendBuff[1] = NextBlockIndex;
        BleSendOneFrame(SendBuff, 2);
    }
    
    return 0;
Error:
    BleSendErrorPacket(CMD_GET_NEXT_SOUND_BLOCK_INDEX_RES, ErrorCode);
    return ErrorCode;   

}

int32_t WriteSoundBlockDataCmdHandler(uint8_t *buff, uint32_t length)
{
    if(IsPasswordPassed() == FALSE)
    {
        return -1;
    }
    
    WriteSoundBlockData(buff, length);

    /*for test*/
//    if(buff[0] == 0xFF)
//    {
//        uint32_t i;
//        for(i = 0; i < 53; i++)
//        {
//            buff[0] = i;
//            WriteSoundBlockData(buff, length);
//        }
//    }
//    else
//    {
//        WriteSoundBlockData(buff, length);
//    }

    
    

    return 0;
}

int32_t CheckSoundBlockDataCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    int32_t LostPacketCnt;
    uint8_t SendBuff[20];
    bool CheckSumValid;
    uint32_t CheckSum;

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }
    
    if(length != 1 && length != 5)
    {
        ErrorCode = ERROR_CHECK_SOUND_BLOCK_DATA_LENGTH | length;
        goto Error;
    }

    
    if(length == 1)
    {
        //旧版本没有校验
        CheckSumValid = FALSE;
        CheckSum = 0;
    }
    else
    {
        //1.5以后的协议增加每个block的数据校验
        CheckSumValid = TRUE;
        memcpy(&CheckSum, buff + 1, 4);
    }
    
    LostPacketCnt = CheckSoundBlockData(buff[0], SendBuff + 1, CheckSum, CheckSumValid);
    if(LostPacketCnt < 0)
    {
        ErrorCode = ERROR_CHECK_SOUND_BLOCK_DATA | (uint16_t)LostPacketCnt;
        goto Error;
    }

    SendBuff[0] = CMD_CHECK_SOUND_BLOCK_DATA_RES;
    BleSendOneFrame(SendBuff, 1 + LostPacketCnt);

    return 0;
Error:
    BleSendErrorPacket(CMD_CHECK_SOUND_BLOCK_DATA_RES, ErrorCode);
    return ErrorCode;   
}

int32_t WriteSoundFinishCmdHandler()
{
    int32_t ret;
    uint32_t ErrorCode;
    uint8_t SendBuff[2];

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }
    
    
    ret = WriteSoundFinish();
    if(ret == 0)
    {
        SendBuff[0] = CMD_WIRTE_SOUND_FINISH_RES;
        BleSendOneFrame(SendBuff, 1);
    }
    else
    {
        ErrorCode = ERROR_WRITE_SOUND_FINISH | (uint16_t)ret;
        goto Error;
    }
    return 0;
Error:
    BleSendErrorPacket(CMD_WIRTE_SOUND_FINISH_RES, ErrorCode);
    return ErrorCode;   

}

int32_t DeleteSoundCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    int32_t DeleteRet;
    uint8_t VehicleIndex;
    uint8_t SoundType;
    uint8_t SendBuff[20];

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }
    

    if(length != 2)
    {
        ErrorCode = ERROR_DELETE_SOUND_LENGTH | length;
        goto Error;
    }
    
    VehicleIndex = buff[0];
    if(VehicleIndex >= VEHICLE_CNT)
    {
        ErrorCode = ERROR_DELETE_SOUND_VEHICLE_INDEX | VehicleIndex;
        goto Error;
    }
 
    SoundType = buff[1];
    if(SoundType > TYPE_ENGINE_SOUND)
    {
        ErrorCode = ERROR_DELETE_SOUND_SOUND_TYPE | SoundType;
        goto Error;
    }

    DeleteRet = DeleteFile(&File[ENGINE_INDEX(VehicleIndex)]);
    if(DeleteRet < 0)
    {
        ErrorCode = ERROR_DELETE_SOUND_SOUND | (uint16_t)DeleteRet;
        goto Error;
    }

    SendBuff[0] = CMD_DELETE_SOUND_RES;
    BleSendOneFrame(SendBuff, 1);
    
    return 0;
Error:
    BleSendErrorPacket(CMD_DELETE_SOUND_RES, ErrorCode);
    return ErrorCode;   
}

int32_t SelectVehicleCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    uint8_t VehicleIndex;
    uint8_t SendBuff[20];
    int32_t ret;

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }
    
    if(length != 1)
    {
        ErrorCode = ERROR_SELECT_SOUND_LENGTH | length;
        goto Error;
    }
    VehicleIndex = buff[0];
    if(VehicleIndex >= VEHICLE_CNT)
    {
        ErrorCode = ERROR_SELECT_SOUND_INDEX | VehicleIndex;
        goto Error;
    }

    //ret = TryChangeSoundIndex(VehicleIndex);
    ret = TryChangeVehileRollBack(VehicleIndex, (bool)FALSE);
    SendBuff[0] = CMD_SELECT_VEHICLE_RES;
    SendBuff[1] = (uint8_t)ret;
    SendBuff[2] = mixer.SoundIndex;

    
    BleSendOneFrame(SendBuff, 3);
    return 0;
    
Error:
    BleSendErrorPacket(CMD_SELECT_VEHICLE_RES, ErrorCode);
    return ErrorCode;       
}

/** 
 * [GetEncKeyCmdHandler description]APP验证PASSWORD前，请求加密密码的RAS公钥和随机码
 * @Author   tin39
 * @DateTime 2019年3月13日T9:28:01+0800
 * @param                             [description]
 * @return                            [description]
 */
int32_t GetRandomCmdHandler()
{
    uint8_t SendBuff[20];
    uint8_t *RandomData;

    //获取8字节随机数
    RandomData = GetPasswordRandom();

    SendBuff[0] = CMD_GET_RANDOM_RES;
    memcpy(SendBuff + 1, RandomData, LENGTH_PASSWORD_RANDOM);
    BleSendOneFrame(SendBuff, 1 + LENGTH_PASSWORD_RANDOM);

    return 0;
}



int32_t PasswordAuthorizeCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    int32_t ret;
    uint8_t SendBuff[2];

    if(length != LENGTH_PASSWORD)
    {
        ErrorCode = ERROR_AUTHORIZATION_CMD_LENGTH | length;
        goto Error;
    }

    ret = AuthorizePassword(buff);
    if(ret != 0)
    {
        //密码验证失败
        ErrorCode = ret;
        goto Error;
    }
    else
    {
        //密码验证成功
        SendBuff[0] = CMD_AUTHORIZATION_RES;
        SendBuff[1] = 0;
        BleSendOneFrame(SendBuff, 2);
    }


    return 0;

Error:
    BleSendErrorPacket(CMD_AUTHORIZATION_RES, ErrorCode);
    return ErrorCode;    
}


int32_t SetPasswordCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    int32_t ret;
    uint8_t SendBuff[2];

    if(IsPasswordPassed() == FALSE)
    {
        ErrorCode = ERROR_PASSWORD_NOT_AUTHORIZED;
        goto Error;
    }

    if(length != LENGTH_PASSWORD)
    {
        ErrorCode = ERROR_AUTHORIZATION_CMD_LENGTH | length;
        goto Error;
    }

    //开始进行密码验证
    ret = SetPassword(buff);
    if(ret != 0)
    {
        //密码验证失败
        ErrorCode = ret;
        goto Error;
    }
    else
    {
        //密码验证成功
        SendBuff[0] = CMD_SET_PASSWORD_RES;
        SendBuff[1] = 0;
        BleSendOneFrame(SendBuff, 2);
    }
    

    return 0;

Error:
    BleSendErrorPacket(CMD_SET_PASSWORD_RES, ErrorCode);
    return ErrorCode;    
}


int32_t ResetPasswordCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    int32_t ret;


    if(length != LENGTH_PASSWORD)
    {
        ErrorCode = ERROR_AUTHORIZATION_CMD_LENGTH | length;
        goto Error;
    }

    
    //开始进行密码重置
    ret = ResetPassword(buff);
    if(ret != 0)
    {
        //密码重置失败
        ErrorCode = ret;
        goto Error;
    }

    return 0;

Error:
    BleSendErrorPacket(CMD_RESET_PASSWORD_RES, ErrorCode);
    return ErrorCode;      

}

int32_t BleDisconnectCmdHandler()
{
    uint8_t SendBuff[1];
    SetPasswordInvalid();
    SendBuff[0] = CMD_BLE_DISCONNECT_RES;
    BleSendOneFrame(SendBuff, 1);
    return 0;
}



void BleAnalysisMessageId(uint8_t *buff, uint32_t length)
{
    switch(buff[0])
    {
        case CMD_SYSTEM_INFO_REQUEST:
            SystemInfoRequestCmdHandler(buff + 1, length - 1);
            break;
        case CMD_CHANGE_PROGRAM:
            ChangeProgeramCmdHandler();
            break;
            
        case CMD_SYSTEM_PARAM_REQUEST:
            SystemParamRequestCmdHandler(buff + 1, length - 1);
            break;
        case CMD_SYSTEM_PARAM_SET:
            SystemParamSetCmdHandler(buff + 1, length - 1);
            break;
        case CMD_SYSTEM_PARAM_STORE:
            SystemParamStoreCmdHandler(buff + 1, length - 1);
            break;

            
        case CMD_VEHICLE_PARAM_REQUEST:
            VehicleParamRequestCmdHandler(buff + 1, length - 1);
            break;
        case CMD_VEHICLE_PARAM_SET:
            VehicleParamSetCmdHandler(buff + 1, length - 1);
            break;
        case CMD_VEHICLE_PARAM_STORE:
            VehicleParamStoreCmdHandler(buff + 1, length - 1);
            break;

        case CMD_FORMAT_FS:
            FormatFsCmdHandler();
            break;
        case CMD_READ_SOUND_GUID:
            ReadSoundGuidCmdHandler(buff + 1, length - 1);
            break;
        case CMD_READ_SOUND_NAME:
            ReadSoundNameCmdHandler(buff + 1, length - 1);
            break;
        case CMD_WRITE_SOUND_START:
            WriteSoundStartCmdHandler(buff + 1, length - 1);
            break;
        case CMD_GET_NEXT_SOUND_BLOCK_INDEX:
            GetNextSoundBlockIndexCmdHandler(buff + 1, length - 1);
            break;
        case CMD_WRITE_SOUND_BLOCK_DATA:
            WriteSoundBlockDataCmdHandler(buff + 1, length - 1);
            break;
        case CMD_CHECK_SOUND_BLOCK_DATA:
            CheckSoundBlockDataCmdHandler(buff + 1, length - 1);
            break;
        case CMD_WIRTE_SOUND_FINISH:
            WriteSoundFinishCmdHandler();
            break;
        case CMD_SET_SOUND_DATA_PACKET_DATA_LENGTH:
            SetDataLengthCmdHandler(buff + 1, length - 1);
            break;
        case CMD_DELETE_SOUND:
            DeleteSoundCmdHandler(buff + 1, length - 1);
            break;
         case CMD_SELECT_VEHICLE:
            SelectVehicleCmdHandler(buff + 1, length - 1);
            break;
        case CMD_GET_RANDOM:
            GetRandomCmdHandler();
            break;
        case CMD_AUTHORIZATION:
            PasswordAuthorizeCmdHandler(buff + 1, length - 1);
            break;
        case CMD_SET_PASSWORD:
            SetPasswordCmdHandler(buff + 1, length - 1);
            break;
        case CMD_RESET_PASSWORD:
            ResetPasswordCmdHandler(buff + 1, length - 1);
            break;
        case CMD_BLE_DISCONNECT:
            BleDisconnectCmdHandler();
            break;
        default:
            break;
    }
}
