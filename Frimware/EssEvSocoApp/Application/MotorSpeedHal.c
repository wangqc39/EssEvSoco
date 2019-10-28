#include "common.h"
#include <stdint.h>
#include "engine.h"
#include "SystemHw.h"
#include "MotorSpeed.h"

#define HAL_SPEED_TIMER			TIM3


uint32_t TotalHalCnt;
uint32_t HalCntPerPeriod;
EXTITrigger_TypeDef  TriggerState;

uint16_t HalCalibrate100KmH;//车速100km/h下，每200ms的HAL脉冲数
uint16_t HalMaxSpeedCalibrate;
uint16_t MaxVehicleCalibrate;//单位km/h
/*
uint16_t WheelDiameter;//单位cm
uint16_t MotorPoles;//电机对极数
uint32_t ReductionRatio;//减速比*100
*/


void MotorSpeedHalHwInit()
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    NVIC_InitTypeDef 	NVIC_InitStructure;
    EXTI_InitTypeDef        EXTI_InitStructure;

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource1);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI1_IRQChannel;//TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    TriggerState = EXTI_Trigger_Rising;
    

    EXTI_ClearITPendingBit(EXTI_Line1);

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
 
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQChannel;//TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
 
    TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
    TIM_TimeBaseStructInit(&TIM_TimeBaseStructure); 
    TIM_TimeBaseStructure.TIM_Period = 20000 - 1;   //44100       
    TIM_TimeBaseStructure.TIM_Prescaler = SYSTEM_CLK / 100000 - 1;       
    TIM_TimeBaseStructure.TIM_ClockDivision = 0x0;    
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(HAL_SPEED_TIMER, &TIM_TimeBaseStructure);

    
    TIM_ClearITPendingBit(HAL_SPEED_TIMER, TIM_IT_Update);
    TIM_ITConfig(HAL_SPEED_TIMER, TIM_IT_Update, ENABLE);

    TIM_Cmd(HAL_SPEED_TIMER, ENABLE);

    
}


void MotorSpeedHalIrqHandler()
{
 //   uint32_t i;
//    EXTI_InitTypeDef        EXTI_InitStructure;
    /*if(TriggerState == EXTI_Trigger_Rising)
    {
        for(i = 0; i < 3; i++)
        {
            if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == Bit_RESET)
                goto exit;
        }

        EXTI_InitStructure.EXTI_Line = EXTI_Line8;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);
        TriggerState = EXTI_Trigger_Falling;
    }
    else
    {
        for(i = 0; i < 3; i++)
        {
            if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_8) == Bit_SET)
                goto exit;
        }

        EXTI_InitStructure.EXTI_Line = EXTI_Line8;
        EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
        EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
        EXTI_InitStructure.EXTI_LineCmd = ENABLE;
        EXTI_Init(&EXTI_InitStructure);
        TriggerState = EXTI_Trigger_Rising;
        TotalHalCnt++;
    }*/

    TotalHalCnt++;
    
//exit:
    EXTI_ClearITPendingBit(EXTI_Line1);
}


void HalSpeedTimerIntHandler()
{
    static uint32_t LastHalCnt;
    uint32_t HalCntPerPeriodTmp;
    TIM_ClearITPendingBit(HAL_SPEED_TIMER, TIM_IT_Update);
    HalCntPerPeriodTmp = TotalHalCnt - LastHalCnt;
    LastHalCnt = TotalHalCnt;

    //HalCntPerPeriod = (HalCntPerPeriod * 3 + HalCntPerPeriodTmp) >> 2;
    HalCntPerPeriod = HalCntPerPeriodTmp;
    
    
}



//uint32_t CurveHallSpeed[2] = {40, 60};//油门曲线斜率变化处对应的霍尔速度
//uint32_t DistSpeedPercent[2];//油门曲线斜率变化处的速度比率

/*
void InitThrottleCurve()
{
    uint32_t percent1, percent2;
    uint32_t PercentInterval;

    percent1 = CurveHallSpeed[0] * 10000 / HalMaxSpeedCalibrate;
    percent2 = CurveHallSpeed[1] * 10000 / HalMaxSpeedCalibrate;
    PercentInterval = percent2 - percent1;

    DistSpeedPercent[0] = percent1 + 1000 * MotorSpeed.ThrottleCurveLevel;//(10000 - percent1) * MotorSpeed.ThrottleCurveLevel / 10;
    if(DistSpeedPercent[0] > 10000)
        DistSpeedPercent[0] = 10000;
        
    DistSpeedPercent[1] = DistSpeedPercent[0] + PercentInterval;
    if(DistSpeedPercent[1] > 10000)
        DistSpeedPercent[1] = 10000;
}
*/


/*
#define HAL_CNT_IDLE				1
//#define HAL_CNT_MAX				60
int16_t GetDistSpeedHal()
{
    int32_t speed;
    int32_t percent;
    int32_t SpeedPercent;
    if(HalCntPerPeriod < HAL_CNT_IDLE)
    {
        speed = engine.ForwardLowSpeed;
    }
    else
    {
        if(HalCntPerPeriod <= CurveHallSpeed[0])
        {
            percent = HalCntPerPeriod * 10000 / CurveHallSpeed[0];
            SpeedPercent = DistSpeedPercent[0] * percent / 10000;
            //speed = engine.ForwardLowSpeed + (engine.ForwardHighSpeed - engine.ForwardLowSpeed) * SpeedPercent / 10000;
        }
        else if(HalCntPerPeriod <= CurveHallSpeed[1])
        {
            percent = (HalCntPerPeriod - CurveHallSpeed[0]) * 10000 / (CurveHallSpeed[1] - CurveHallSpeed[0]);
            SpeedPercent = DistSpeedPercent[0] + (DistSpeedPercent[1] - DistSpeedPercent[0]) * percent / 10000;
            if(SpeedPercent > 10000)
                SpeedPercent = 10000;
            
        }
        else
        {
            if(HalMaxSpeedCalibrate == CurveHallSpeed[1])
            {
                SpeedPercent = 10000;
                
            }
            else
            {
                percent = (HalCntPerPeriod - CurveHallSpeed[1]) * 10000 / (HalMaxSpeedCalibrate - CurveHallSpeed[1]);
                SpeedPercent = DistSpeedPercent[1] + (10000 - DistSpeedPercent[1]) * percent / 10000;
                if(SpeedPercent > 10000)
                    SpeedPercent = 10000;
            }
            
        }
        speed = engine.ForwardLowSpeed + (engine.ForwardHighSpeed - engine.ForwardLowSpeed) * SpeedPercent / 10000;
    }
    
    //speed = GestEvDistSpeed(speed);
    return (int16_t)speed;
}
*/

#define CHAN_MAX_VALUE 10000

#define KMAX 100


int expou(int x, unsigned short int k)
{
	// k*x*x*x + (1-k)*x
	// 0 <= k <= 100

	unsigned int val = (x * x / CHAN_MAX_VALUE * x / CHAN_MAX_VALUE * k
		+ (KMAX - k) * x + KMAX / 2) / KMAX;

	return val;
}

int expo(int value, int k_Ratio)
{
	int  y;
	int  k;

	unsigned neg = value < 0;

	k = neg ? k_Ratio : k_Ratio;


	if (k == 0)
		return value;

	if (neg)
		value = -value; //absval

	if (k < 0)
	{
		y = CHAN_MAX_VALUE - expou(CHAN_MAX_VALUE - value, -k);
	}
	else
	{
		y = expou(value, k);
	}

	return neg ? -y : y;
}

void CalHalMaxSpeedCalibrate()
{
/*
    uint64_t temp;
    uint64_t VehicleSpeed;
    uint64_t perimeter;//周长
    temp = MotorPoles * 2;//磁极对乘2，磁极个数。表示电机机械转转动一圈，在一个霍尔信号线上触发的信号周期个数
    temp = temp * ReductionRatio;//计算车轮机械旋转一圈的信号个数。计算齿比后放大100倍。ToothRatio为齿比* 100；第一个100用于对冲ToothRatio放大的100倍，第二个100，用于放大，提高精度。
    VehicleSpeed = MaxVehicleCalibrate * 100 * 10 / 36 ;//从KM/H到M/S换算，结果放大100倍，也可理解为CM/S
    perimeter = (WheelDiameter * 314);//计算周长，单位m * 10000
    HalMaxSpeedCalibrate = temp * VehicleSpeed / perimeter / 5; //除以5是因为200ms获取一次数据，temp放大100倍，VehicleSpeed放大100倍，perimeter放大10000倍，正好抵消
*/

    //根据百公里车速下HAL的标定值和标定的最大车速进行计算，得出声音最大速度对应的HAL标定值
    HalMaxSpeedCalibrate = (uint32_t)HalCalibrate100KmH * MaxVehicleCalibrate / 100;
}


int16_t GetDistSpeedHal()
{
    int32_t speed;
    int32_t ThPercent;
    int32_t SpeedPercent;
    ThPercent = (HalCntPerPeriod * 10000) / HalMaxSpeedCalibrate;
    if(ThPercent > 10000)
        ThPercent = 10000;
    SpeedPercent = expo(ThPercent, -MotorSpeed.ThrottleCurveLevel * 10);
    speed = engine.ForwardLowSpeed + (engine.ForwardHighSpeed - engine.ForwardLowSpeed) * SpeedPercent / 10000;
    return (int16_t)speed;
}

/*
int16_t GetDistSpeedHal()
{
    int32_t speed;
    int32_t percent;
    if(HalCntPerPeriod < HAL_CNT_IDLE)
    {
        speed = engine.ForwardLowSpeed;
    }
    else
    {
        percent = HalCntPerPeriod - HAL_CNT_IDLE;
        percent = (percent * 10000) / (HalMaxSpeedCalibrate - HAL_CNT_IDLE);
        if(percent > 10000)
            percent  = 10000;
        speed = engine.ForwardLowSpeed + (engine.ForwardHighSpeed - engine.ForwardLowSpeed) * percent / 10000;
    }
    
    //speed = GestEvDistSpeed(speed);
    return (int16_t)speed;
}
*/




