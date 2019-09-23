#include "common.h"
#include "ActionTick.h"
#include "nrf52.h"
#include "TestModel.h"

volatile uint32_t SystemTick;




void SystickConfig(void)
{
  SysTick_Config(64000000 / 10000);
}

void SysTick_Handler(void)
{
    SystemTick++;
}

uint32_t GetSystemTime()
{
    return SystemTick;
}

void mDelay(int ms)
{
    volatile uint32_t InTime;
    InTime = GetSystemTime();
    while(GetSystemTime() - InTime <= MS(ms))
    {
        //fwdgt_counter_reload();
    }
}


