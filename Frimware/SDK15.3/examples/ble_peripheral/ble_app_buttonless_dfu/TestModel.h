#ifndef __TEST_MODEL__
#define __TEST_MODEL__
#include "nrf_gpio.h"


//#define PIN_DEBUG           25
//#define DEBUG_PIN_H         nrf_gpio_pin_set(PIN_DEBUG)
//#define DEBUG_PIN_L         nrf_gpio_pin_clear(PIN_DEBUG)

void MeasureOneCycleMinTime(void);
void CalCpuLoad(void);
void InitTestModelTimer(void);




#endif

