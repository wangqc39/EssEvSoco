#ifndef __STSTEMHWCONFIG__
#define  __STSTEMHWCONFIG__


typedef enum {BOOT_APP = 0, BOOT_BOOTLOADER = 1} BootFromTypeInfo;

void Init_All_Periph(void);
void StopWorking(void);
void IwdgInit(void); 
void SystemStandByHandler(void);
BootFromTypeInfo BootCheck(void);
void SetBootData(BootFromTypeInfo BootData);


extern u32 SystemClk;

#endif
