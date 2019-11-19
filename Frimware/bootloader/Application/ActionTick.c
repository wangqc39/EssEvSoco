#include "main.h"

//每100ns加1
volatile u32 SystemTick = 0;


static void SystemTickConfiguration()
{
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);

    SysTick->LOAD = SystemClk / 8000;

    SysTick->CTRL |= ((u32)0x00000002);
    SysTick->CTRL |= ((u32)0x00000001);
    //SysTick_ITConfig(ENABLE);
    //SysTick_CounterCmd(SysTick_Counter_Disable);
    NVIC_SystemHandlerPriorityConfig(SystemHandler_SysTick, 3, 3);  
}


void ActionTickHwInit()
{
    SystemTickConfiguration();
}

//没1ms进行按键事件处理
void SystemTickInterruptHandler()
{
    SystemTick++;
}

u32 GetSystemTime()
{
    return SystemTick;
}


void mDelay(int ms)
{
    volatile unsigned int InTime;
    InTime = GetSystemTime();
    while(GetSystemTime() - InTime <= ms)
    {
        IWDG_ReloadCounter();
    }
}

void uDelay(int us)
{
    volatile int i, j;
    for(i = 0; i < us; i++)
        for(j = 0; j < 0x5; j++);
}

