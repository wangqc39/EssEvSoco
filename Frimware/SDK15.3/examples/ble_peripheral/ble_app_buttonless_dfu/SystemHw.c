#include "ActionTick.h"
//#include "nrf_delay.h"
//#include "fs.h"

#include "SystemInfo.h"
#include "SpiFlash.h"
#include "ActionTick.h"
#include "key.h"
#include "mixer.h"
#include "engine.h"
#include "OnChipLed.h"

//#include "BleComHw.h"
#include "MotorSpeed.h"

#include "MotorSpeedHal.h"

#include "nrf_drv_gpiote.h"
#include "TestModel.h"




//设置NFC关闭
const uint32_t UICR_ADDR_0x20C    __attribute__((at(0x1000120C))) __attribute__((used)) = 0xFFFFFFFE;


void InitAllPeriph(void)
{
    uint32_t err_code;
    //ItIsASecret();
    //备份寄存器设定，用于启动时指定驻留在BOOTLOADER还是APP
    //SetBootData(BOOT_APP);
    

    SystickConfig();


    
    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);
    //
    OnChipLedHwInit();
    //
    ////FLASH初始化
    SpiFlashHwInit();

    KeyHwInit();



    AudioOutHwConfig();



    EvMotorSpeedHwInit();


    InitTestModelTimer();
    //TestHwInit();


    MotorSpeedHalHwInit();
    
}




