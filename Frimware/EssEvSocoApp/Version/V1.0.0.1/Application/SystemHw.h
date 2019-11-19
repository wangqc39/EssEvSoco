#ifndef __STSTEMHWCONFIG__
#define  __STSTEMHWCONFIG__

typedef enum {BOOT_APP = 0, BOOT_BOOTLOADER = 1} BootFromTypeInfo;

#define STM32_PAGE_SIZE                         (0x800)

#define SYSTEM_CLK				64000000

void InitAllPeriph(void);
void RcPlusConnectMonitor(void);
void SetBootData(BootFromTypeInfo BootData);
void IwdgInit(void);




#endif
