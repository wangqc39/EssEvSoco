#include "Common.h"
#include "OnChipLed.h"
#include "SystemError.h"
#include "ActionTick.h"

#include "nrf_gpio.h"

#define PIN_LED                 27

//表示错误状态时，LED闪烁的间隔
#define ERROR_STATUS_LED_FLICKER_INTERVAL	2500

#define ON_CHIP_LED_OFF			nrf_gpio_pin_set(PIN_LED)//GPIO_SetBits(GPIOC, GPIO_Pin_3);
#define ON_CHIP_LED_ON			nrf_gpio_pin_clear(PIN_LED)//GPIO_ResetBits(GPIOC, GPIO_Pin_3);



void OnChipLedHwInit()
{
    //nrf_gpio_cfg_output(PIN_LED);
    nrf_gpio_cfg(
        PIN_LED,
        NRF_GPIO_PIN_DIR_OUTPUT,
        NRF_GPIO_PIN_INPUT_DISCONNECT,
        NRF_GPIO_PIN_NOPULL,
        NRF_GPIO_PIN_S0D1,
        NRF_GPIO_PIN_NOSENSE);

}

void OnChipLedHandler()
{
    static uint8_t LedStatus = 0;
    static uint32_t LastFlickTime;
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


