#include "nrf52.h"
#include "nrf_drv_timer.h"
#include "nrf_gpio.h"
#include "TestModel.h"


const nrf_drv_timer_t TEST_MODEL_TIMER = NRF_DRV_TIMER_INSTANCE(1);  


uint32_t thisCycles, lastCycles;
uint32_t minCycles = 0xFFFFFFFF;
uint32_t idleCounter, oldIdleCounter;
uint32_t LastLastCycle;
float idlePercent;


void TestModelTimeIntHandlerNull(nrf_timer_event_t event_type, void* p_context)
{
    idlePercent = (float)(idleCounter - oldIdleCounter) / (16000000 / minCycles);
    oldIdleCounter = idleCounter;
}


void InitTestModelTimer()
{
    uint32_t err_code = NRF_SUCCESS;
    nrf_drv_timer_config_t timer_cfg;// = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    timer_cfg.frequency = NRF_TIMER_FREQ_16MHz;
    timer_cfg.interrupt_priority = 6;
    timer_cfg.mode = NRF_TIMER_MODE_TIMER;
    timer_cfg.p_context = NULL;
    err_code = nrf_drv_timer_init(&TEST_MODEL_TIMER, &timer_cfg, TestModelTimeIntHandlerNull);
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_extended_compare(
         &TEST_MODEL_TIMER, NRF_TIMER_CC_CHANNEL0, 16000000, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);


    nrf_drv_timer_enable(&TEST_MODEL_TIMER);    
}

/** 
 * [MeasureOneCycleMinTime description]在主循环whlie中调用，计算一次while循环最短的时间
 * @Author   tin39
 * @DateTime 2019年7月22日T14:20:44+0800
 * @param                             [description]
 */
void MeasureOneCycleMinTime()
{
    uint32_t cycles;
    TEST_MODEL_TIMER.p_reg->TASKS_CAPTURE[1] = 1;
    thisCycles = TEST_MODEL_TIMER.p_reg->CC[1];
    if(thisCycles < lastCycles)
    {
        cycles = 0xFFFFFFFF + thisCycles - lastCycles;
    }
    else
    {
        cycles = thisCycles - lastCycles;
    }
    LastLastCycle = lastCycles;
    lastCycles = thisCycles; 

    //得到在这个while中循环的最短时间，近似认为是没有其他中断，纯粹地在这个while中运行的指令周期
    if (cycles < minCycles)
        minCycles = cycles;


    idleCounter++;
}


//void TestHwInit()
//{
//    nrf_gpio_cfg_output(PIN_DEBUG);
//}




//void CalCpuLoad()
//{
//    //每秒计算一次CPU负荷
//    if(SystemTick % 100000 == 0)
//    {
//        //64000000 / minCycles：计算如果CPU完全空闲时，1秒内运行while循环的次数
//        //idleCounter - oldIdleCounter：实际1秒内，运行while的次数
//        
//    }
//}




