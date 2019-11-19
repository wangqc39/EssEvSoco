#include "main.h"
#include "string.h"
#include "SystemInfo.h"
#include "ActionTick.h"


struct SystemsInfo SystemInformation;

//计算设备的序列号
void GetDeviceId()
{
    vu32 ID0, ID2, ID1;
    unsigned char *DistributorIdAddr;
    ID0 = *(vu32*)(ID0_ADDR);
    ID1 = *(vu32*)(ID1_ADDR);
    ID2 = *(vu32*)(ID2_ADDR);

    //temp = ID0;
    //异或为可逆处理，同时又能保证异或后的数据仍旧是全球唯一的
    ID0 = ID0 ^ 0x55aaaa55;
    ID1 = ID1 ^ 0xaa5555aa;
    ID2 = ID2 ^ 0x669977ee;

    SystemInformation.DeviceId[0] = (u8)(ID2 >> 24);
    SystemInformation.DeviceId[1] = (u8)(ID1 >> 24);
    SystemInformation.DeviceId[2] = (u8)(ID0 >> 24);
    SystemInformation.DeviceId[3] = (u8)(ID0 >> 16);
    SystemInformation.DeviceId[4] = (u8)(ID1 >> 16);
    SystemInformation.DeviceId[5] = (u8)(ID2 >> 16);
    SystemInformation.DeviceId[6] = (u8)(ID2 >> 8);
    SystemInformation.DeviceId[7] = (u8)ID1;
    SystemInformation.DeviceId[8] = (u8)(ID0 >> 8);
    SystemInformation.DeviceId[9] = (u8)ID0;
    SystemInformation.DeviceId[10] = (u8)(ID1 >> 8);
    SystemInformation.DeviceId[11] = (u8)ID2;
    DistributorIdAddr = (unsigned char *)(SYSTEM_INFO_ADDR);
    SystemInformation.DeviceId[12] = *DistributorIdAddr++;
    SystemInformation.DeviceId[13] = *DistributorIdAddr++;
    SystemInformation.DeviceId[14] = *DistributorIdAddr++;
    SystemInformation.DeviceId[15] = *DistributorIdAddr++;
    if(SystemInformation.DeviceId[12] == 0xFF && SystemInformation.DeviceId[13] == 0xFF && 
       SystemInformation.DeviceId[14] == 0xFF && SystemInformation.DeviceId[15] == 0xFF)
    {
        //如果经销商ID数据未处理，则返回为0
        SystemInformation.DeviceId[12] = 0;
        SystemInformation.DeviceId[13] = 0;
        SystemInformation.DeviceId[14] = 0;
        SystemInformation.DeviceId[15] = 0;
    }    
}





//获取一个系统信息
//返回值为系统信息的内容长度
//如果Index错误，返回-1	
int32_t GetOneSystemInfo(uint8_t *buff, uint8_t Index)
{
    int32_t DataLength;
    uint32_t TimeMs;
    uint32_t Tmp;
    switch(Index)
    {
        case 1:
            buff[0] = (uint8_t)PRODUCT_TYPE;
            buff[1] = PRODUCT_TYPE >> 8;
            DataLength = 2;
            break;
        case 2:
            buff[0] = PROTOCAL_VERSION;
            DataLength = 1;
            break;
        case 3:
            memcpy(buff, MODEL, sizeof(MODEL));
            DataLength = sizeof(MODEL);
            break;
        case 4:
            memcpy(buff, MANUFACTURE, sizeof(MANUFACTURE));
            DataLength = sizeof(MANUFACTURE);
            break;
        case 5:
            memcpy(buff, HARDWARE_VERSION, sizeof(HARDWARE_VERSION));
            DataLength = sizeof(HARDWARE_VERSION);
            break;
        case 6:
            buff[0] = 0;
            DataLength = 1;
            break;
        case 7:
            buff[0] = BOOTLOADER_VERSION_1;
            buff[1] = BOOTLOADER_VERSION_2;
            buff[2] = BOOTLOADER_VERSION_3;
            buff[3] = (uint8_t)BOOTLOADER_VERSION_4;
            buff[4] = BOOTLOADER_VERSION_4 >> 8;
            DataLength = 5;
            break;
        case 8:
            memset(buff, 0, 5);
            DataLength = 5;
            break;
        case 9:
            memcpy(buff, SystemInformation.DeviceId, 16);
            DataLength = 16;
            break;
        case 10:
            TimeMs = GetSystemTime();
            memcpy(buff, &TimeMs, 4);
            DataLength = 4;
            break;
        case 11:
            //Todo
            Tmp = 8 * 1024 * 1024;
            memcpy(buff, &Tmp, 4);
            DataLength = 4;
            break;
        case 12:
            //Todo
            Tmp = 0;
            memcpy(buff, &Tmp, 4);
            DataLength = 4;
            break;
        case 13:
            //Todo
            buff[0] = (uint8_t)BLOCK_SIZE_DATA_TRANSFER;
            buff[1] = BLOCK_SIZE_DATA_TRANSFER >> 8;
            DataLength = 2;
            break;
        case 14:
            buff[0] = 0;
            DataLength = 1;
            break;
        default:
            return -1;
    }
    return DataLength;
}



