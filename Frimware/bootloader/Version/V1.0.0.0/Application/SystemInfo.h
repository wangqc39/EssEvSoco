#ifndef __SYSTEM_INFO__
#define __SYSTEM_INFO__

#include "main.h"






#define BOOTLOADER_VERSION_1			1
#define BOOTLOADER_VERSION_2			0
#define BOOTLOADER_VERSION_3			0
#define BOOTLOADER_VERSION_4			0



#define MAX_INDEX_SYSTEM_INFO		14

#define PRODUCT_TYPE				0x0A06
#define PROTOCAL_VERSION			1
#define MODEL							"ESS EV SOCO"
#define MANUFACTURE					"SenseHobby Inc."
#define HARDWARE_VERSION			"AAA"
#define BOOTLOADER_VERSION			0

#define BLOCK_SIZE_DATA_TRANSFER	1024
#define VEHICLE_CNT					4



struct SystemsInfo
{
    unsigned char DeviceId[16];
    FunctionalState MassStorageWriteEnableFlag;
};


extern struct SystemsInfo SystemInformation;

void GetDeviceId(void);
int CheckSystemConfigVersion(unsigned int *ConfigVersion);
void ItIsASecret(void);
int IsDeviceProduced(void);


int32_t GetOneSystemInfo(uint8_t *buff, uint8_t Index);


#endif

