#include "Common.h"
#include "OnChipLed.h"
#include "SystemError.h"
#include "ActionTick.h"

//表示错误状态时，LED闪烁的间隔
#define ERROR_STATUS_LED_FLICKER_INTERVAL	2500




void OnChipLedHwInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;

    ON_CHIP_LED_OFF;
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

}

void OnChipLedHandler()
{
    static u8 LedStatus = 0;
    static u32 LastFlickTime;
    if(SystemError.MostSeriousError == NO_ERROR)
    {
        ON_CHIP_LED_ON;
    }
    else
    {
        if(GetSystemTime() > LastFlickTime + ERROR_STATUS_LED_FLICKER_INTERVAL)
        {
            LastFlickTime = GetSystemTime();
            if(LedStatus == 0)
            {
                LedStatus = 1;
                ON_CHIP_LED_ON;
            }
            else
            {
                LedStatus = 0;
                ON_CHIP_LED_OFF;
            }
        }
    }
}
