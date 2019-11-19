#include "main.h"
#include <stdint.h>
#include <string.h>
#include "SystemInfo.h"
#include "BleTransportLayer.h"
#include "DownLoader.h"
#include "BleError.h"


#define CMD_SYSTEM_INFO_REQUEST			0x10
#define CMD_SYSTEM_INFO_REQUEST_RES		CMD_SYSTEM_INFO_REQUEST + 0x80

#define CMD_CHANGE_PROGRAM				0x5A
#define CMD_CHANGE_PROGRAM_RES			CMD_CHANGE_PROGRAM + 0x80

#define CMD_WRITE_FIRMWARE_START        0x50
#define CMD_WRITE_FIRMWARE_START_RES    CMD_WRITE_FIRMWARE_START + 0x80

#define CMD_GET_NEXT_FIRMWARE_BLOCK_INDEX       0x51
#define CMD_GET_NEXT_FIRMWARE_BLOCK_INDEX_RES   CMD_GET_NEXT_FIRMWARE_BLOCK_INDEX + 0x80

#define CMD_WRITE_FIRMWARE_BLOCK_DATA           0x70

#define CMD_CHECK_FIRMWARE_BLOCK_DATA           0x52
#define CMD_CHECK_FIRMWARE_BLOCK_DATA_RES       CMD_CHECK_FIRMWARE_BLOCK_DATA + 0x80

#define CMD_WRITE_FIRMWARE_FINISH               0x53
#define CMD_WRITE_FIRMWARE_FINISH_RES           CMD_WRITE_FIRMWARE_FINISH + 0x80





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


    for(i = 0; i < length; i++)
    {
        index = buff[i];
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

    return 0;
}


int32_t ChangeProgeramCmdHandler()
{
    uint8_t SendBuff[20];
    SendBuff[0] = CMD_CHANGE_PROGRAM_RES;
    BleSendOneFrame(SendBuff, 1);

    mDelay(10);

    //Todo:程序切换
    RunToApplication(APPLICATION_ADDRESS);
    
    return 0;
}




int32_t WriteFirmwareStartCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    int32_t ret;
    uint8_t SendBuff[20];
    uint32_t FirmwareSize;
    uint32_t CheckSum;
    DownloadModeInfo DownloadMode;
    
    if(length != 9)
    {
        ErrorCode = ERROR_WRITE_FIRMWARE_START_LENGTH | (uint16_t)length;
        goto Error;
    }
    
    memcpy(&FirmwareSize, buff, sizeof(uint32_t));
    if(FirmwareSize > 200 * 1024 || FirmwareSize < 10 * 1024)
    {
        ErrorCode = ERROR_WRITE_FIRMWARE_START_FIRMWARE_SIZE;
        goto Error;
    }

    if(FirmwareSize % 16 != 0)
    {
        ErrorCode = ERROR_WRITE_FIRMWARE_START_FIRMWARE_SIZE_ALIGN;
        goto Error;
    }

    memcpy(&CheckSum, buff + 4, sizeof(uint32_t));
    memcpy(&DownloadMode, buff + 8, sizeof(uint8_t));

    if(DownloadMode > DOWNLOAD_MODE_REDOWNLOAD)
    {
        ErrorCode = ERROR_WRITE_FIRMWARE_START_FIRMWARE_MODE | (uint8_t)DownloadMode;
        goto Error;
    }
    
    
    ret = WriteFirmwareStart(FirmwareSize, CheckSum, DownloadMode);
    if(ret == 0)
    {
        SendBuff[0] = CMD_WRITE_FIRMWARE_START_RES;
        BleSendOneFrame(SendBuff, 1);
    }
    else
    {
        ErrorCode = ERROR_WRITE_FIRMWARE_START | (uint16_t)ret;
        goto Error;
    }
    
    return 0;
Error:
    BleSendErrorPacket(CMD_WRITE_FIRMWARE_START_RES, ErrorCode);
    return ErrorCode;   
}


int32_t GetNextFirmwareBlockIndexCmdHandler(uint8_t *buff, uint32_t length)
{
    int32_t NextBlockIndex;
    uint8_t SendBuff[2];
    uint32_t ErrorCode;
    NextBlockIndex = GetNextFirmwareBlockIndex();
    if(NextBlockIndex < 0)
    {
        ErrorCode = ERROR_GET_NEXT_FIRMWARE_BLOCK_INDEX | (uint16_t)NextBlockIndex;
        BleSendErrorPacket(CMD_GET_NEXT_FIRMWARE_BLOCK_INDEX_RES, ErrorCode);
        return NextBlockIndex;
    }

    
    SendBuff[0] = CMD_GET_NEXT_FIRMWARE_BLOCK_INDEX_RES;
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
}

int32_t WriteFirmwareBlockDataCmdHandler(uint8_t *buff, uint32_t length)
{
    WriteFirmwareBlockData(buff, length);
    return 0;
}

int32_t CheckFirmwareBlockDataCmdHandler(uint8_t *buff, uint32_t length)
{
    uint32_t ErrorCode;
    int32_t LostPacketCnt;
    uint8_t SendBuff[20];
    if(length != 1)
    {
        ErrorCode = ERROR_CHECK_FIRMWARE_BLOCK_DATA_LENGTH | length;
        goto Error;
    }
    
    LostPacketCnt = CheckFirmwareBlockData(buff[0], SendBuff + 1);
    if(LostPacketCnt < 0)
    {
        ErrorCode = ERROR_CHECK_FIRMWARE_BLOCK_DATA | (uint16_t)LostPacketCnt;
        goto Error;
    }

    SendBuff[0] = CMD_CHECK_FIRMWARE_BLOCK_DATA_RES;
    BleSendOneFrame(SendBuff, 1 + LostPacketCnt);

    return 0;
Error:
    BleSendErrorPacket(CMD_CHECK_FIRMWARE_BLOCK_DATA_RES, ErrorCode);
    return ErrorCode;   
}

int32_t WriteFirmwareFinishCmdHandler()
{
    int32_t ret;
    uint32_t ErrorCode;
    uint8_t SendBuff[2];
    
    
    ret = WriteFirmwareFinish();
    if(ret == 0)
    {
        SendBuff[0] = CMD_WRITE_FIRMWARE_FINISH_RES;
        BleSendOneFrame(SendBuff, 1);
    }
    else
    {
        ErrorCode = ERROR_WRITE_FIRMWARE_FINISH | (uint16_t)ret;
        BleSendErrorPacket(CMD_WRITE_FIRMWARE_FINISH_RES, ErrorCode);
    }
    return ret;
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
            
        
        case CMD_WRITE_FIRMWARE_START:
            WriteFirmwareStartCmdHandler(buff + 1, length - 1);
            break;
        case CMD_GET_NEXT_FIRMWARE_BLOCK_INDEX:
            GetNextFirmwareBlockIndexCmdHandler(buff + 1, length - 1);
            break;
        case CMD_WRITE_FIRMWARE_BLOCK_DATA:
            WriteFirmwareBlockDataCmdHandler(buff + 1, length - 1);
            break;
        case CMD_CHECK_FIRMWARE_BLOCK_DATA:
            CheckFirmwareBlockDataCmdHandler(buff + 1, length - 1);
            break;
        case CMD_WRITE_FIRMWARE_FINISH:
            WriteFirmwareFinishCmdHandler();
            break;

        default:
            break;
    }
}
