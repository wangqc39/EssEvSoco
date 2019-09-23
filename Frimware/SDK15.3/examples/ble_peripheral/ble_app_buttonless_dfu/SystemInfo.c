#include <nrfx.h>

#include "common.h"
#include "string.h"
#include "SystemInfo.h"
#include "SystemError.h"
#include "ActionTick.h"


struct SystemsInfo SystemInformation;





//查询设备是否注册过
//0:未注册
// 1:已注册
#define DEVICE_SERIAL_NUM_OFFSET			116
int IsDeviceProduced()
{
    int i;
    unsigned char *ptr =  (unsigned char *)(SYSTEM_INFO_ADDR);
    for(i = 0; i < 32; i++)
    {
        if(*ptr++ != 0xFF)
        {
            return 1;
        }
    }

    return 0;
}


//计算设备的序列号
void GetDeviceId()
{
    volatile uint32_t ID0, ID2, ID1;
    unsigned char *DistributorIdAddr;
    //ID0 = *(volatile uint32_t*)(ID0_ADDR);
    //ID1 = *(volatile uint32_t*)(ID1_ADDR);
    //ID2 = *(volatile uint32_t*)(ID2_ADDR);
    ID0 = NRF_FICR->DEVICEID[0];
    ID1 = NRF_FICR->DEVICEID[1];
    ID2 = ID0 ^ ID1;

    //temp = ID0;
    //异或为可逆处理，同时又能保证异或后的数据仍旧是全球唯一的
    ID0 = ID0 ^ 0x55aaaa55;
    ID1 = ID1 ^ 0xaa5555aa;
    ID2 = ID2 ^ 0x669977ee;

    SystemInformation.DeviceId[0] = (uint8_t)(ID2 >> 24);
    SystemInformation.DeviceId[1] = (uint8_t)(ID1 >> 24);
    SystemInformation.DeviceId[2] = (uint8_t)(ID0 >> 24);
    SystemInformation.DeviceId[3] = (uint8_t)(ID0 >> 16);
    SystemInformation.DeviceId[4] = (uint8_t)(ID1 >> 16);
    SystemInformation.DeviceId[5] = (uint8_t)(ID2 >> 16);
    SystemInformation.DeviceId[6] = (uint8_t)(ID2 >> 8);
    SystemInformation.DeviceId[7] = (uint8_t)ID1;
    SystemInformation.DeviceId[8] = (uint8_t)(ID0 >> 8);
    SystemInformation.DeviceId[9] = (uint8_t)ID0;
    SystemInformation.DeviceId[10] = (uint8_t)(ID1 >> 8);
    SystemInformation.DeviceId[11] = (uint8_t)ID2;
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

#define  SECRET_ADDRESS		(SECURE_START_ADDR + OFFSET_SECRET)
static unsigned char C1[] = {"Ess Ev"};//20
static unsigned char C2[] = {"Awesome!"};//22

void ItIsASecret()
{
    unsigned char C3[] = {"I hope ererything goes well."};//23
    volatile uint32_t ID0, ID2, ID1;
    volatile uint8_t *ptr;
    volatile uint32_t *PtrTemp;
    volatile uint32_t i, j;
    volatile uint32_t DataTemp;
    uint32_t temp;
    uint32_t ReadResult[3];
    unsigned char FlashId[12] = {"just  change"};
    //ID0 = *(volatile uint32_t*)(ID0_ADDR);
    //ID1 = *(volatile uint32_t*)(ID1_ADDR);
    //ID2 = *(volatile uint32_t*)(ID2_ADDR);
    ID0 = NRF_FICR->DEVICEID[0];
    ID1 = NRF_FICR->DEVICEID[1];
    ID2 = ID0 ^ ID1;

    ptr = (uint8_t *)C1;
    for(i = 0; i < sizeof(C1) / 4; i++)
    {
        DataTemp = *ptr++;
        DataTemp |= (*ptr++ << 8);
        DataTemp |= (*ptr++ << 16);
        DataTemp |= (*ptr++ << 24);
        ID0 ^= DataTemp;
    }

    ptr = (uint8_t *)C2;
    for(i = 0; i < sizeof(C2) / 4; i++)
    {
        DataTemp = *ptr++;
        DataTemp |= (*ptr++ << 8);
        DataTemp |= (*ptr++ << 16);
        DataTemp |= (*ptr++ << 24);
        ID1 ^= DataTemp;
    }

    ptr = (uint8_t *)C3;
    for(i = 0; i < sizeof(C3) / 4; i++)
    {
        DataTemp = *ptr++;
        DataTemp |= (*ptr++ << 8);
        DataTemp |= (*ptr++ << 16);
        DataTemp |= (*ptr++ << 24);
        ID2 ^= DataTemp;
    }

    temp = ID2;
    PtrTemp = (uint32_t *)FlashId;
    ID2 = ID1 - *PtrTemp++;
    ID1 = ID0 + *PtrTemp++;
    ID0 = temp ^ *PtrTemp;

    
    ReadResult[0] = *(volatile uint32_t*)SECRET_ADDRESS;
    ReadResult[1] = *(volatile uint32_t*)(SECRET_ADDRESS + 4);
    ReadResult[2] = *(volatile uint32_t*)(SECRET_ADDRESS + 8);


    if(ReadResult[2] != ID0 || ReadResult[1] != ID1 || ReadResult[0] != ID2)
    {
        while(1);
    }
    

    
}


int CheckSystemConfigVersion(unsigned int *ConfigVersion)
{
    if(*ConfigVersion != 0xFFFFFFFF)
    {
        if(*ConfigVersion != ESS_ONE_PLUS_CONFIG_VERSION)
        {
            SetSystmError(CONFIG_VERSION_ERROR);
        }
    }
    else
    {
        *ConfigVersion = ESS_ONE_PLUS_CONFIG_VERSION;
        return -1;
    }
    return 0;
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
            buff[0] = 1;
            DataLength = 1;
            break;
        case 7:
            memset(buff, 0, 5);
            DataLength = 5;
            break;
        case 8:
            buff[0] = FIRMWARE_VERSION_1;
            buff[1] = FIRMWARE_VERSION_2;
            buff[2] = FIRMWARE_VERSION_3;
            buff[3] = (uint8_t)FIRMWARE_VERSION_4;
            buff[4] = FIRMWARE_VERSION_4 >> 8;
            DataLength = 5;
            break;
        case 9:
            memcpy(buff, SystemInformation.DeviceId, 16);
            DataLength = 16;
            break;
        case 10:
            TimeMs = GetSystemTime() / TIMER_MULTI;
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
            buff[0] = VEHICLE_CNT;
            DataLength = 1;
            break;
        default:
            return -1;
    }
    return DataLength;
}



