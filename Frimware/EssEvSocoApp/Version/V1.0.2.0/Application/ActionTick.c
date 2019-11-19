#include "common.h"
#include "key.h"


//每100ns加1
volatile unsigned int SystemTick;


void SystemTickConfiguration(void)
{
    /* Disable SysTick Counter */
    SysTick_CounterCmd(SysTick_Counter_Disable);
  
    /* Disable the SysTick Interrupt */
    SysTick_ITConfig(DISABLE);
  
    /* Configure HCLK clock as SysTick clock source */
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);  //时钟除于8

    SysTick_SetReload(800);  //计数周期长度
   
    SysTick_CounterCmd(SysTick_Counter_Enable);   //启动定时器 
    SysTick_ITConfig(ENABLE);  //打开中断

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
    volatile u32 InTime;
    InTime = GetSystemTime();
    while(GetSystemTime() - InTime <= ms * 10)
    {
        KeyStatusHandler();
        IWDG_ReloadCounter();
    }
}

void uDelay(int us)
{
    volatile int i, j;
    for(i = 0; i < us; i++)
        for(j = 0; j < 0x5; j++);
}


