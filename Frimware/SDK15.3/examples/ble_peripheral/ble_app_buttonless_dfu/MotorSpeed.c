#include "common.h"
#include "MotorSpeed.h"
#include "Engine.h"
#include "ActionTick.h"


#include "MotorSpeedHal.h"
#include "nrf_saadc.h"
#include "nrf_gpio.h"
#include "nrf_ppi.h"


#define PIN_HOLD                3

#define ADC_CHANNEL_TH          0
#define ADC_INPUT_TH          NRF_SAADC_INPUT_AIN4
#define ADC_TH_GAIN_SET       NRF_SAADC_GAIN1_6
#define ADC_TH_GAIN             6



//当引擎达到最高速并持续一段时间后，速度会缓慢回落
//这个值表示回落后的数据占最高速的百分比
#define EV_HOLD_SPEED			80  
//当速度超过HOLD速度后，保持该时间，然后速度缓慢下降到HOLD速度
#define EV_OVER_HOLD_TIME		2000 * TIMER_MULTI

//速度从OverHold到HOLD过程中，速度每下降0.1的时间间隔，单位ms
#define EV_SPEED_TO_HOLD_RATE    5 * TIMER_MULTI

//速度一二挡时，前进限速是最大速度的百分比
#define EV_SPEED1_SPEED_PERCENT					64
#define EV_SPEED2_SPEED_PERCENT					80

//电动车加速度比率，HOLD状态时/电机转动时
//#define EV_ACC_RATIO_NO_HOLD					10




#define EV_ACC_RATIO_SPEED1					10
#define EV_ACC_RATIO_SPEED2					9
#define EV_ACC_RATIO_SPEED3					8







#define EV_HOLD_BUTTON			nrf_gpio_pin_read(PIN_HOLD)//GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9)
//#define EV_GEAR_SWITCH1			GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)
//#define EV_GEAR_SWITCH2			GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_9)


                         
//刹车检测时间，在该时间内检测油门变化
#define BREAK_IN_AREA_TIME_GATE			500  //单位ms

//在最大档位时，最高转速的百分比
//最大档位时，一般不会到达引擎的最大转速，需要限定ESS能够到达的最大转速
#define MAX_SPEED_PERCENT_IN_MAX_GEAR				80  //单位%




//换挡油门的位置百分比
#define UP_GEAR_THROTTLE_PERCENT			95
#define DOWN_GEAR_THROTTLE_PERCENT		50




struct MotorSpeedInfo MotorSpeed = 
{
.EvAccRatio = PERCENT_BASE,
.GearBox.GearBoxEnableFlag = DISABLE,
.EvSpeedLevel = EV_SPEED_HOLD
};


struct TurnInfo Turn;


/** 
 * [AdcContinueModeInit description]进行TH ADC的初始化，通过2路PPI使ADC采样自动化，设置完成后进行持续采样，采样结果放置到内存中
 * @Author   tin39
 * @DateTime 2019年7月17日T14:03:13+0800
 * @param                             [description]
 */
void AdcContinueModeInit()
{
    
    nrf_ppi_channel_endpoint_setup(PPI_CHANNEL_ADC_DONE_TO_START, (uint32_t)((uint8_t *)NRF_SAADC + (uint32_t)NRF_SAADC_EVENT_DONE),
                                       (uint32_t)((uint8_t *)NRF_SAADC + (uint32_t)NRF_SAADC_TASK_START));
    nrf_ppi_channel_endpoint_setup(PPI_CHANNEL_ADC_START_TO_SAMPLE, (uint32_t)((uint8_t *)NRF_SAADC + (uint32_t)NRF_SAADC_EVENT_STARTED),
                                       (uint32_t)((uint8_t *)NRF_SAADC + (uint32_t)NRF_SAADC_TASK_SAMPLE));
    nrf_ppi_channel_enable(PPI_CHANNEL_ADC_DONE_TO_START);
    nrf_ppi_channel_enable(PPI_CHANNEL_ADC_START_TO_SAMPLE);


    //采用ADC的continue模式，该模式只能有1路ADC，多路ADC需要使用其他方法实现
    nrf_saadc_resolution_set(NRF_SAADC_RESOLUTION_12BIT);
    nrf_saadc_oversample_set(NRF_SAADC_OVERSAMPLE_DISABLED);
    nrf_saadc_buffer_init((nrf_saadc_value_t *)&Turn.TurnAdcValue, 1);
    

    
    nrf_saadc_channel_config_t saadc_channel_config = 
    {
        .resistor_p = NRF_SAADC_RESISTOR_DISABLED,
        .resistor_n = NRF_SAADC_RESISTOR_DISABLED,
        .gain = ADC_TH_GAIN_SET,
        .reference = NRF_SAADC_REFERENCE_INTERNAL,
        .acq_time = NRF_SAADC_ACQTIME_40US,
        .mode = NRF_SAADC_MODE_SINGLE_ENDED,
        .burst = NRF_SAADC_BURST_DISABLED,
        .pin_p = ADC_INPUT_TH,
        .pin_n = NRF_SAADC_INPUT_DISABLED
    };
    nrf_saadc_channel_init(ADC_CHANNEL_TH, &saadc_channel_config);
    //nrf_saadc_continuous_mode_enable(2047);//令内部采集频率最低，采集频率：16M/2047

    nrf_saadc_enable();

    nrf_saadc_event_clear(NRF_SAADC_EVENT_STARTED);

    nrf_saadc_task_trigger(NRF_SAADC_TASK_START);

    //while(nrf_saadc_event_check(NRF_SAADC_EVENT_STARTED) == false);

    //nrf_saadc_task_trigger(NRF_SAADC_TASK_SAMPLE);
}



void EvMotorSpeedHwInit()
{
    AdcContinueModeInit();
    




    nrf_gpio_cfg_input(PIN_HOLD, NRF_GPIO_PIN_NOPULL);
    //GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
    //GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    //GPIO_Init(GPIOA, &GPIO_InitStructure);

/*
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
*/
}


#define TURN_UP_RESISTANCE		1000//1000
//#define TURN_UP_RESISTANCE		1000

#define TURN_DOWN_RESISTANCE	1000
#define ADC_STANDARD_VOLTAGE			600
int16_t GetTurnVoltage()
{
    int32_t Temp;
    int32_t CurrentVoltage;
    static int32_t LastVoltage = 0;
    //计算电压值，精确到小数点后3位									  e
    Temp = ((int32_t)Turn.TurnAdcValue * (TURN_DOWN_RESISTANCE + TURN_UP_RESISTANCE)) / TURN_DOWN_RESISTANCE;
    CurrentVoltage = (Temp * ADC_STANDARD_VOLTAGE * ADC_TH_GAIN) >> 12;



    CurrentVoltage = (CurrentVoltage + LastVoltage * 15) / 16;
    LastVoltage = CurrentVoltage;
    return CurrentVoltage;
}




//EvSpeedLevelInfo EvSpeedLevel;
//EvSpeedLevelInfo New;
uint8_t P_Level;
void EvMotorSpeedHandler()
{
    static uint32_t LastHandleTime;
    //static 
    EvSpeedLevelInfo ThisEvSpeedLevel;

    if(GetSystemTime() < LastHandleTime + 500 * TIMER_MULTI)
        return;

    LastHandleTime = GetSystemTime();

    P_Level = EV_HOLD_BUTTON;

    if(EV_HOLD_BUTTON != 0)
    {
        //移除了档位的线
        ThisEvSpeedLevel = EV_SPEED_HOLD;
    }
    else
    {
           
        ThisEvSpeedLevel = EV_SPEED3;
    }
    //ThisEvSpeedLevel = EV_SPEED_HOLD;
/*
    else if(EV_GEAR_SWITCH1 == Bit_SET && EV_GEAR_SWITCH2 == Bit_SET)
    {
        ThisEvSpeedLevel = EV_SPEED2;
    }
    else if(EV_GEAR_SWITCH2 == Bit_RESET)
    {
        ThisEvSpeedLevel = EV_SPEED3;
    }
    else
    {
        ThisEvSpeedLevel = EV_SPEED1;
    }
*/

    //if(ThisEvSpeedLevel == MotorSpeed.EvSpeedLevel)
    //    return;
    MotorSpeed.EvSpeedLevel = ThisEvSpeedLevel;

//   if(New == EvSpeedLevel)
//        return;
//   EvSpeedLevel = New;
//   ThisEvSpeedLevel = New;

   
    MotorSpeed.EvMaxSpeed = engine.ForwardHighSpeed;

    if(ThisEvSpeedLevel == EV_SPEED_HOLD)
    {
        MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseP * 4) / 10 / 10000;
    }
    else
    {
        //HAL实时转速模式下，如果架起摩托车空载，响应太快会导致声音台阶状态，故适当降低响应
        MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseP * 4) / 10 / 3 / 10000;
    }
    
    
    //根据速度档位，设定当前档位的最大速度及响应曲线
    /*if(ThisEvSpeedLevel == EV_SPEED_HOLD)
    {
        MotorSpeed.EvMaxSpeed = engine.ForwardHighSpeed;
        MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseP * 4) / 10 / 10000;
    }
    else if(ThisEvSpeedLevel == EV_SPEED1)
    {
        MotorSpeed.EvMaxSpeed = (int32_t)engine.ForwardHighSpeed * EV_SPEED1_SPEED_PERCENT / 100;
        if(MotorSpeed.SpeedMode == SPEED_MODE_HAL)
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseD * 4) / 10 / 10000;
        }
        else
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * (uint32_t)engine.EngineAccResponseD  * 4) / 10 / EV_ACC_RATIO_SPEED1 / 10000;
        }
    }
    else if(ThisEvSpeedLevel == EV_SPEED2)
    {
        MotorSpeed.EvMaxSpeed = (int32_t)engine.ForwardHighSpeed * EV_SPEED2_SPEED_PERCENT / 100;
        if(MotorSpeed.SpeedMode == SPEED_MODE_HAL)
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseD * 4) / 10 / 10000;
        }
        else
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * (uint32_t)engine.EngineAccResponseD * 4) / 10 / EV_ACC_RATIO_SPEED2 / 10000;
        }
    }
    else
    {
        MotorSpeed.EvMaxSpeed = engine.ForwardHighSpeed;
        if(MotorSpeed.SpeedMode == SPEED_MODE_HAL)
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * engine.EngineAccResponseD * 4) / 10 / 10000;
        }
        else
        {
            MotorSpeed.EvAccRatio = (PERCENT_BASE * (uint32_t)engine.EngineAccResponseD * 4) / 10 / EV_ACC_RATIO_SPEED3 / 10000;
        }
    }*/
    //根据最大速度进行HOLD速度的计算
    MotorSpeed.EvHoldSpeed = engine.ForwardLowSpeed + ((int32_t)((int32_t)MotorSpeed.EvMaxSpeed - engine.ForwardLowSpeed) * EV_HOLD_SPEED) / 100; 
}

//#endif




//#ifndef PROTOCAL_DD
/******************绝对目标速度计算模块*************************/
int16_t GestEvDistSpeed(int16_t DistSpeedRaw)
{
    static bool SpeedOverHoldFlag = false;//之前速度是否超过AFTER_MAX_SPEED_ENGINE_DOWN_PERCENT的标志
    static uint32_t OverHoldTime = 0;
    static uint32_t LastHoldSpeedFlushTime;//在HOLD速度干预过程中，速度更新的时刻
    static int16_t LastHoldSpeed;//速度在HOLD区间内超过指定时间后，对速度进行干预时的当前速度
    
    int16_t DistSpeed;

    if(SpeedOverHoldFlag == false)
    {
        //之前速度在HOLD之下
        if(DistSpeedRaw >= MotorSpeed.EvHoldSpeed && MotorSpeed.RealSpeed >= DistSpeedRaw)
        {
            //速度超过HOLD，记录进入HOLD区域的时刻
            OverHoldTime = GetSystemTime();
            SpeedOverHoldFlag = true;
        }
        DistSpeed = DistSpeedRaw;
    }
    else
    {
        if(DistSpeedRaw < MotorSpeed.EvHoldSpeed)
        {
            //速度回落到HOLD以下，不进行速度干预
            DistSpeed = DistSpeedRaw;
            SpeedOverHoldFlag = false;
        }
        else
        {
            //速度在HOLD区域内
            if(GetSystemTime() < OverHoldTime + EV_OVER_HOLD_TIME)
            {
                //刚刚进入HOLD区域，对于速度不做干涉和限制
                DistSpeed = DistSpeedRaw;
                //记录当前的速度，用于后面速度干预的计算
                LastHoldSpeed = DistSpeedRaw;
                LastHoldSpeedFlushTime = GetSystemTime();
            }
            else
            {
                //进入HOLD的时间超过EV_OVER_HOLD_TIME，进行速度干预
                if(GetSystemTime() < LastHoldSpeedFlushTime + EV_SPEED_TO_HOLD_RATE)
                {
                    //尚未到达一次数据更新时间
                    DistSpeed = LastHoldSpeed;
                }
                else
                {
                    //到达数据干预的时间，进行一次速度干预
                    DistSpeed = LastHoldSpeed - ((MotorSpeed.EvMaxSpeed - MotorSpeed.EvHoldSpeed) / (100 - EV_HOLD_SPEED)) / 10;
                    //如果已经下降到达HOLD速度，则锁定到HOLD速度
                    if(DistSpeed < MotorSpeed.EvHoldSpeed)
                        DistSpeed = MotorSpeed.EvHoldSpeed;
                    LastHoldSpeed = DistSpeed;
                    LastHoldSpeedFlushTime = GetSystemTime();
                }
            }
        }        
    }
    return DistSpeed;
}
//#endif


int16_t GetDistSpeedWithoutGearBox(int16_t TurnVoltage)
{
    int32_t speed;
    int32_t percent;
    if(TurnVoltage < VOLTAGE_TURN_IDLE)
    {
        speed = engine.ForwardLowSpeed;
    }
    else
    {
        percent = TurnVoltage - VOLTAGE_TURN_IDLE;
        percent = (percent * 10000) / (VOLTAGE_TURN_MAX - VOLTAGE_TURN_IDLE);
        if(percent > 10000)
            percent  = 10000;
        speed = engine.ForwardLowSpeed + (MotorSpeed.EvMaxSpeed - engine.ForwardLowSpeed) * percent / 10000;
    }

/*
    //在P档下，声音不再回落
    if(MotorSpeed.EvSpeedLevel != EV_SPEED_HOLD)
    {
        //在D档，声音回落
        speed = GestEvDistSpeed(speed);
    }
*/
    
    return (int16_t)speed;
}
	

//目标:得到目标速度和油门区间位置
//若满足减档条件，则减档
int16_t GetDistSpeedWithGearBox(uint16_t TurnVoltage)
{
    int32_t speed;
    int32_t percent;
    if(TurnVoltage < MotorSpeed.GearBox.DownGearThrottlePosition)
    {
        //油门处于减档区间
        if(TurnVoltage < VOLTAGE_TURN_IDLE)
        {
            speed = engine.ForwardLowSpeed;
        }
        else
        {
            percent = TurnVoltage - VOLTAGE_TURN_IDLE;
            percent = (percent * 10000) / (MotorSpeed.GearBox.UpGearThrottlePosition - VOLTAGE_TURN_IDLE);
            if(percent > 10000)
                percent  = 10000;
            speed = engine.ForwardLowSpeed + (MotorSpeed.GearBox.ChangeGearSpeed - engine.ForwardLowSpeed) * percent / 10000;
        }
        MotorSpeed.GearBox.ThrottlePosition = DOWN_GEAR_THROTTLE_POSITION;
    }
    else if(TurnVoltage < MotorSpeed.GearBox.UpGearThrottlePosition)
    {
        //中间区域，不升档也不降档
        percent = TurnVoltage - VOLTAGE_TURN_IDLE;
        percent = (percent * 10000) / (MotorSpeed.GearBox.UpGearThrottlePosition - VOLTAGE_TURN_IDLE);
        if(percent > 10000)
            percent  = 10000;
        speed = engine.ForwardLowSpeed + (MotorSpeed.GearBox.ChangeGearSpeed - engine.ForwardLowSpeed) * percent / 10000;
        MotorSpeed.GearBox.ThrottlePosition = HOLD_GEAR_THROTTLE_POSITION;
    }
    else
    {
        //油门在升档区间
        if(MotorSpeed.GearBox.GearBoxStatus == GEAR_UPING)
        {
            //若在升档降速过程中，把速度设定为最低速,为的是升档降速过程的平滑
            speed = engine.ForwardLowSpeed;	
        }
        else
        {
            if(MotorSpeed.GearBox.GeraLevel == MotorSpeed.GearBox.MaxGearLevel)
            {
                /*//若已升到最高档位，则油门指示实际目标速度
                //目标速度的在升档区间的最低值通过齿比算出
                PwmDifference = (int32_t)((int32_t)MotorSpeed.HighCycle - MotorSpeed.GearBox.ChangeGearThrottlePosition);
                PwmDifference = PwmDifference * 10000 / (SystemConfig.ForwardHighPwm - MotorSpeed.GearBox.ChangeGearThrottlePosition);
                //该区间速度为升档后的速度到最高速
                Speed = MotorSpeed.GearBox.AfterUpGearSpeed + (SystemConfig.ForwardHighSpeed - MotorSpeed.GearBox.AfterUpGearSpeed) * PwmDifference / 10000;*/
                percent = TurnVoltage - VOLTAGE_TURN_IDLE;
                percent = (percent * 10000) / (VOLTAGE_TURN_MAX - VOLTAGE_TURN_IDLE);
                if(percent > 10000)
                    percent  = 10000;
                speed = engine.ForwardLowSpeed + (engine.ForwardHighSpeed * MAX_SPEED_PERCENT_IN_MAX_GEAR / 100 - engine.ForwardLowSpeed) * percent / 10000;
                
            }
            else
            {
                //未升到最高档，则油门指示目标速度为最高速度
                //把速度设定为最高速，进行升档
                speed = engine.ForwardHighSpeed;
            }
            
        }
        MotorSpeed.GearBox.ThrottlePosition = UP_GEAR_THROTTLE_POSITION;
    }
    return (int16_t)speed;
}






/*************************实际速度计算*******************************/
//变速箱比例基数及其位移值
//#define GEAR_RATIO_BASE_SHIFT			13
//#define GEAR_RATIO_BASE					(1 << GEAR_RATIO_BASE_SHIFT)
//p值的倒数，用来做除法运算
#define KP_FACTOR					1000		
#define THORTTLE_MIN_FACTOR			100
#define THORTTLE_MAX_FACTOR		800
#define KP_SPEED_DOWN_PERCENT		1
#define KP_UPGEAR_DOWN_PERCENT	2
uint16_t Acc;
int16_t DoAccelerateWithMotorCurve(int16_t NowSpeed, int16_t Error, uint32_t GearRatio)
{
    int16_t DistError;
    int i;
    int32_t Accelerate;
    //uint16_t AccelerateLow;
    //uint16_t AccelerateHigh;
    int32_t percent;
    int32_t temp;
    int32_t ErrorFactor;

    

    if(NowSpeed >= 0)
    {
    
        if(Error > 0)
        {
            //得出速度在哪个区间范围内
            for(i = 0; i < engine.ForwardAudioFileNum; i++)
            {
                if(NowSpeed <= engine.ForwardSpeedArray[i])
                {
                    break;
                }
            }
    
            //根据速度计算当下的加速力矩
            if(i == 0)
            {
                //若发动机速度小于最小速度，则直接采用怠速时的力矩
                Accelerate = (int32_t)engine.AccelerateArrayOri[0] * GearRatio;
                Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
            }
            else if(i >= engine.ForwardAudioFileNum)
            {
                //若发动机速度大于发动机最大速度，则直接采用发动机最大速度时的力矩
                Accelerate = (int32_t)engine.AccelerateArrayOri[engine.ForwardAudioFileNum - 1] * GearRatio;
                Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
            }
            else
            {
                //中间阶段，通过百分比来计算力矩的值
                if(engine.AccelerateArrayOri[i] == engine.AccelerateArrayOri[i - 1])
                {
                    //若相邻的两个加速度力矩相同，则无需计算
                    Accelerate = (int32_t)engine.AccelerateArrayOri[i] * GearRatio;
                    Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
                }
                else
                {
                    //根据百分比计算当下的加速度力矩
                    percent = (int32_t)((int32_t)(NowSpeed - engine.ForwardSpeedArray[i - 1]) * PERCENT_BASE);
                    percent = percent / (engine.ForwardSpeedArray[i] - engine.ForwardSpeedArray[i - 1]);
                    if(engine.AccelerateArrayOri[i] > engine.AccelerateArrayOri[i - 1])
                    {
                        Accelerate = (int32_t)((int32_t)(engine.AccelerateArrayOri[i] - engine.AccelerateArrayOri[i - 1]) * percent);
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i - 1];
                    }
                    else
                    {
                        Accelerate = (int32_t)((int32_t)(engine.AccelerateArrayOri[i - 1] - engine.AccelerateArrayOri[i]) * (PERCENT_BASE - percent)); 
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i];                       
                    }
                    Accelerate = Accelerate * GearRatio;
                    Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
                }
    
            }
    
            //根据加速度的力矩和速度差值(油门大小)来计算增加的速度
            percent = (int32_t)(Error << PERCENT_BASE_SHIFT) / engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 1];
            ErrorFactor = THORTTLE_MIN_FACTOR + (((THORTTLE_MAX_FACTOR - THORTTLE_MIN_FACTOR) * percent) >> PERCENT_BASE_SHIFT);
            temp = ErrorFactor *  Accelerate;
            DistError = temp / KP_FACTOR;

            Acc = Accelerate;
        }
        else
        {
            if(Error < -3000)
                Error = -3000;
            if(MotorSpeed.GearBox.GearBoxStatus == NORMAL_DRIVE || MotorSpeed.GearBox.GearBoxStatus == GEAR_DOWN)
            {
                DistError = (int16_t)((Error * KP_SPEED_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
            }
            else
            {
                DistError = (int16_t)((Error * KP_UPGEAR_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
            }
        }

        
    }
    else
    {
        if(Error < 0)
        {
            NowSpeed = 0 - NowSpeed;
           //得出速度在哪个区间范围内
            for(i = 0; i < engine.BackwardAudioFileNum; i++)
            {
                if(NowSpeed <= engine.BackwardSpeedArray[i])
                {
                    break;
                }
            }
    
            //根据速度计算当下的加速力矩
            if(i == 0)
            {
                //若发动机速度小于最小速度，则直接采用怠速时的力矩
                Accelerate = engine.AccelerateArrayOri[0] * GearRatio;
                Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
            }
            else if(i >= engine.BackwardAudioFileNum)
            {
                //若发动机速度大于发动机最大速度，则直接采用发动机最大速度时的力矩
                Accelerate = engine.AccelerateArrayOri[engine.BackwardAudioFileNum - 1] * GearRatio;
                Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
            }
            else
            {
                //中间阶段，通过百分比来计算力矩的值
                if(engine.AccelerateArrayOri[i] == engine.AccelerateArrayOri[i - 1])
                {
                    //若相邻的两个加速度力矩相同，则无需计算
                    Accelerate = engine.AccelerateArrayOri[i] * GearRatio;
                    Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
                }
                else
                {
                    //根据百分比计算当下的加速度力矩
                    percent = (int32_t)((int32_t)(engine.BackwardSpeedArray[i - 1] - NowSpeed) * PERCENT_BASE);
                    percent = percent / (engine.BackwardSpeedArray[i - 1] - engine.BackwardSpeedArray[i]);
                    if(engine.AccelerateArrayOri[i] > engine.AccelerateArrayOri[i - 1])
                    {
                        Accelerate = (int32_t)((int32_t)(engine.AccelerateArrayOri[i] - engine.AccelerateArrayOri[i - 1]) * percent);
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i - 1];
                    }
                    else
                    {
                        Accelerate = (int32_t)((int32_t)(engine.AccelerateArrayOri[i - 1] - engine.AccelerateArrayOri[i]) * (PERCENT_BASE - percent)); 
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i];
                    }
                    Accelerate = Accelerate * GearRatio;
                    Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
                }
    
            }
    
            //根据加速度的力矩和速度差值(油门大小)来计算增加的速度
            percent = (int32_t)(Error << PERCENT_BASE_SHIFT) / engine.BackwardSpeedArray[engine.BackwardAudioFileNum - 1];
            ErrorFactor = (0 - THORTTLE_MIN_FACTOR) + (((THORTTLE_MAX_FACTOR - THORTTLE_MIN_FACTOR) * percent) >> PERCENT_BASE_SHIFT);
            temp = ErrorFactor *  Accelerate;
            DistError = temp / KP_FACTOR;
        }
        else
        {
            if(Error < -3000)
                Error = -3000;
            if(MotorSpeed.GearBox.GearBoxStatus == NORMAL_DRIVE || MotorSpeed.GearBox.GearBoxStatus == GEAR_DOWN)
            {
                DistError = (int16_t)((Error * KP_SPEED_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
            }
            else
            {
                DistError = (int16_t)((Error * KP_UPGEAR_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
            }
            
        }
    }
    return DistError;
}

//通过强制控制RealSpeed来控制是否播放过载声音
int16_t EngineOverloadHandler(int16_t RealSpeed)
{
    static uint32_t InOverloadSpeedTime = 0;

    if(RealSpeed > engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 2])
    {
        //进入过载判断
        if(InOverloadSpeedTime == 0)
        {
            //速度开始上升到过载速度，记录下时间点
            InOverloadSpeedTime = GetSystemTime();
            RealSpeed = engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 2];
        }
        else
        {
            //油门持续在过载区间
            if(GetSystemTime() - InOverloadSpeedTime >= engine.OverloadDelayTime * TIMER_MULTI)
            {
                //可以播放过载的声音，速度进入到过载区间
            }
            else
            {
                RealSpeed = engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 2];
            }
        }
    }
    else
    {
        //退出过载记录及判断
        InOverloadSpeedTime = 0;
        //OverloadPlayFlag = DISABLE;
    }

    return RealSpeed;
    
}

int16_t RealSpeedOtherFilter(int16_t RealSpeed)
{
    //进行发动机过载声音的相关处理
    if(engine.OverloadExistFlag == ENABLE && engine.OverloadEnableFlag == ENABLE)
    {
        RealSpeed = EngineOverloadHandler(RealSpeed);
    }

    //如果发动机是单向的，则实际速度小于0时，限制在速度0
    if(RealSpeed < 0)
    {
        RealSpeed = 0;
    }


    //进行刹车的相关处理
    if(MotorSpeed.VehicleStatu != BREAK)
    {
        if(RealSpeed >= 0)
        {
            //怠速区域，均视为前进
            MotorSpeed.VehicleStatu = GO_FORWARD;
        }
        else
        {
            MotorSpeed.VehicleStatu = GO_BACKWARD;
        }
    }
    else
    {
    }

    return RealSpeed;
}



int16_t GetMotorRealSpeedWithoutGearbox(int16_t DistSpeed, int16_t RealSpeed)
{
    int16_t Error;
    int16_t Adjust;
    
    //static int16_t RealSpeed = 0;
    /*if(MotorSpeed.RealSpeedChangeFlag == 1)
    {
        MotorSpeed.RealSpeedChangeFlag = 0;
        RealSpeed = MotorSpeed.RealSpeed;
    }*/
    
    //计算得到初步的实际速度
    Error = DistSpeed - RealSpeed;
    //有偏差进行调整
    if(Error)
    {
        //Adjust = DoAccelerateWithMotorCurve(RealSpeed, Error, PERCENT_BASE);
        Adjust = DoAccelerateWithMotorCurve(RealSpeed, Error, (uint32_t)(MotorSpeed.EvAccRatio >> REAL_SPEED_CAL_MULT_SHIFT));
        if(Adjust)
        {
            RealSpeed = Adjust + RealSpeed;
        }
        else
        {
            if(Error > 0 )
                RealSpeed = RealSpeed + 1;
            else
                RealSpeed = RealSpeed - 1;
        }
    }

    
    RealSpeed = RealSpeedOtherFilter(RealSpeed);
    
    return RealSpeed;
    
}


int16_t GetMotorRealSpeedWithGearBox(int16_t DistSpeed, int16_t RealSpeed)
{
    int16_t Error;
    int16_t Adjust;
    uint16_t EngineAcc;
    
    //static int16_t RealSpeed = 0;

    /*if(MotorSpeed.RealSpeedChangeFlag == 1)
    {
        //和启动有关，当播放完启动声音后设置该标识，重置下本函数中的RealSpeed的变量
        MotorSpeed.RealSpeedChangeFlag = 0;
        RealSpeed = MotorSpeed.RealSpeed;
    }*/

    Error = DistSpeed - RealSpeed;
    //有偏差进行调整
    if(Error)
    {
        EngineAcc = ((uint32_t)MotorSpeed.GearBox.Ratio[MotorSpeed.GearBox.GeraLevel] * engine.EngineAccResponseP) / 10000;
        Adjust = DoAccelerateWithMotorCurve(RealSpeed, Error, EngineAcc);
        if(Adjust)
        {
            RealSpeed = Adjust + RealSpeed;
        }
        else
        {
            if(Error > 0 )
                RealSpeed = RealSpeed + 1;
            else
                RealSpeed = RealSpeed - 1;
        }
    }

    if(MotorSpeed.GearBox.GearBoxStatus == NORMAL_DRIVE)
    {
        //正常模式下，声音随着油门的变化而变化
        if(MotorSpeed.GearBox.ThrottlePosition == UP_GEAR_THROTTLE_POSITION)
        {
            //进入加档
            MotorSpeed.GearBox.GearBoxStatus = GEAR_UP;
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == DOWN_GEAR_THROTTLE_POSITION)
        {
            if(MotorSpeed.GearBox.GeraLevel > 1)
            {
                //一档以上，进行降档
                MotorSpeed.GearBox.GearBoxStatus = GEAR_DOWN;
                MotorSpeed.GearBox.InGearDownTime = GetSystemTime();
            }
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == REVERSE_THROTTLE_POSITION)
        {
            //油门进入反向区间，则直接减到1档
            MotorSpeed.GearBox.GeraLevel = 1;
        }
    }
    else if(MotorSpeed.GearBox.GearBoxStatus == GEAR_DOWN)
    {
        //在降档状态中
        if(MotorSpeed.GearBox.ThrottlePosition == REVERSE_THROTTLE_POSITION)
        {
            //油门进入反向区间，直接减到1档，进入正常运行模式
            MotorSpeed.GearBox.GeraLevel = 1;
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == UP_GEAR_THROTTLE_POSITION)
        {
            //进入升档
            MotorSpeed.GearBox.GearBoxStatus = GEAR_UP;
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == HOLD_GEAR_THROTTLE_POSITION)
        {
            //进入正常模式
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
        }
        else
        {
            if(GetSystemTime() - MotorSpeed.GearBox.InGearDownTime >= MotorSpeed.GearBox.DownGearDelayTime * TIMER_MULTI)
            {
                //降档延时达到，进行降档，若降到了1档则进入正常模式
                MotorSpeed.GearBox.GeraLevel--;
                MotorSpeed.GearBox.InGearDownTime = GetSystemTime();
                if(MotorSpeed.GearBox.GeraLevel == 1)
                {
                    //若降到了1档，则进行降档
                    MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
                }
            }
        }
    }
    else if(MotorSpeed.GearBox.GearBoxStatus == GEAR_UP)
    {
        //升档状态中
        if(MotorSpeed.GearBox.ThrottlePosition == REVERSE_THROTTLE_POSITION)
        {
            //油门反向，直接降为1档
            MotorSpeed.GearBox.GeraLevel = 1;
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == DOWN_GEAR_THROTTLE_POSITION)
        {
            //若不是最低档则降档，否则进入正常模式
            if(MotorSpeed.GearBox.GeraLevel > 1)
            {
                //一档以上，进行降档
                MotorSpeed.GearBox.GearBoxStatus = GEAR_DOWN;
                MotorSpeed.GearBox.InGearDownTime = GetSystemTime();
            }
            else
            {
                MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            }
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == HOLD_GEAR_THROTTLE_POSITION)
        {
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
        }
        else
        {
            if(RealSpeed >= MotorSpeed.GearBox.ChangeGearSpeed  && MotorSpeed.GearBox.GeraLevel < MotorSpeed.GearBox.MaxGearLevel)
            {
                //速度超过升档速度，进行升档前的等待
                
                MotorSpeed.GearBox.GearBoxStatus = GEAR_UP_WAITING;
                MotorSpeed.GearBox.InGearUpTime = GetSystemTime();
            }
        }
    }
    else if(MotorSpeed.GearBox.GearBoxStatus == GEAR_UPING)
    {
        if(MotorSpeed.GearBox.ThrottlePosition == REVERSE_THROTTLE_POSITION)
        {
            MotorSpeed.GearBox.GeraLevel = 1;
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == DOWN_GEAR_THROTTLE_POSITION)
        {
            if(MotorSpeed.GearBox.GeraLevel > 1)
            {
                //一档以上，进行降档
                MotorSpeed.GearBox.GearBoxStatus = GEAR_DOWN;
                MotorSpeed.GearBox.InGearDownTime = GetSystemTime();
            }
            else
            {
                MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            }
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == HOLD_GEAR_THROTTLE_POSITION)
        {
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
        }
        else 
        {
            if(RealSpeed <= MotorSpeed.GearBox.AfterUpGearSpeed)
            {
                //油门保持，速度降到相应齿比下的转速

                    //未到最高档，继续升档
                    MotorSpeed.GearBox.GearBoxStatus =  GEAR_UP;
                    MotorSpeed.GearBox.GeraLevel++;
            }
        }
    }
    else if(MotorSpeed.GearBox.GearBoxStatus == GEAR_UP_WAITING)
    {
        if(MotorSpeed.GearBox.ThrottlePosition == REVERSE_THROTTLE_POSITION)
        {
            MotorSpeed.GearBox.GeraLevel = 1;
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == DOWN_GEAR_THROTTLE_POSITION)
        {
            if(MotorSpeed.GearBox.GeraLevel > 1)
            {
                //一档以上，进行降档
                MotorSpeed.GearBox.GearBoxStatus = GEAR_DOWN;
                MotorSpeed.GearBox.InGearDownTime = GetSystemTime();
            }
            else
            {
                MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
            }
        }
        else if(MotorSpeed.GearBox.ThrottlePosition == HOLD_GEAR_THROTTLE_POSITION)
        {
            MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
        }
        else 
        {
            if(GetSystemTime() > MotorSpeed.GearBox.InGearUpTime + MotorSpeed.GearBox.UpGearDelayTime * TIMER_MULTI)
            {
                //升档前等待时间到，进行升档
                MotorSpeed.GearBox.GearBoxStatus =  GEAR_UPING;
            }
        }
    }
    
    //在升档过程中，若速度大于升档速度，则强制在升档速度，防止出现过载声音
    if(MotorSpeed.GearBox.GeraLevel != MotorSpeed.GearBox.MaxGearLevel)
    {
        if(RealSpeed > MotorSpeed.GearBox.ChangeGearSpeed)
        {
            RealSpeed = MotorSpeed.GearBox.ChangeGearSpeed;
        }
    }


    RealSpeed = RealSpeedOtherFilter(RealSpeed);

    return RealSpeed;
}


/************************变速箱状态初始化**********************/
void GearBoxSpeedReset()
{
    //最高速度作为换挡速度
    if(engine.OverloadEnableFlag == ENABLE && engine.OverloadExistFlag == ENABLE)
    {
        MotorSpeed.GearBox.ChangeGearSpeed = (int32_t)engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 2] * MotorSpeed.GearBox.UpGearSpeedPercent / 100;
    }
    else
    {
        MotorSpeed.GearBox.ChangeGearSpeed = (int32_t)engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 1] * MotorSpeed.GearBox.UpGearSpeedPercent / 100;
    }

    //设定换挡完成后发动机应用的速度
    MotorSpeed.GearBox.AfterUpGearSpeed = (int32_t)MotorSpeed.GearBox.ChangeGearSpeed * MotorSpeed.GearBox.AfterUpGearSpeedPercent / 100;
    //如果设定太低，比怠速低，导致换档达不到怠速声，从而换挡后无法加速
    if(MotorSpeed.GearBox.AfterUpGearSpeed < engine.ForwardLowSpeed)
    {
        MotorSpeed.GearBox.AfterUpGearSpeed = engine.ForwardLowSpeed;
    }
}


void GearBoxThrottlePositionReset()
{
/*
    if(RcCapture.ForwardHighPwm > RcCapture.MiddleThrottlePwm)
    {
       MotorSpeed.GearBox.UpGearThrottlePosition = (uint32_t)(RcCapture.ForwardHighPwm - RcCapture.MiddleThrottlePwm) 
                                                                                     * UP_GEAR_THROTTLE_PERCENT / 100 + RcCapture.MiddleThrottlePwm;
       MotorSpeed.GearBox.DownGearThrottlePosition = (uint32_t)(RcCapture.ForwardHighPwm - RcCapture.MiddleThrottlePwm) 
                                                                                     * DOWN_GEAR_THROTTLE_PERCENT / 100 + RcCapture.MiddleThrottlePwm;                                                                              
    }
    else
    {
        MotorSpeed.GearBox.UpGearThrottlePosition = (uint32_t)(RcCapture.MiddleThrottlePwm - RcCapture.ForwardHighPwm) 
                                                                                     * (100 - UP_GEAR_THROTTLE_PERCENT) / 100 + RcCapture.ForwardHighPwm;
        MotorSpeed.GearBox.DownGearThrottlePosition = (uint32_t)(RcCapture.MiddleThrottlePwm - RcCapture.ForwardHighPwm) 
                                                                                     * (100 - DOWN_GEAR_THROTTLE_PERCENT) / 100 + RcCapture.ForwardHighPwm;         
    }
*/
    MotorSpeed.GearBox.UpGearThrottlePosition = ((uint32_t)(VOLTAGE_TURN_MAX - VOLTAGE_TURN_IDLE) * UP_GEAR_THROTTLE_PERCENT) / 100 + VOLTAGE_TURN_IDLE;
    MotorSpeed.GearBox.DownGearThrottlePosition = ((uint32_t)(VOLTAGE_TURN_MAX - VOLTAGE_TURN_IDLE) * DOWN_GEAR_THROTTLE_PERCENT) / 100 + VOLTAGE_TURN_IDLE; 
}

/*
void GetGearBoxAccelerateArray()
{
    int i;
    int AccelerateArrayFactor = 40;
    for(i = 0; i < MAX_AUDIO_FILE_NUM_ONE_DIRECT; i++)
    {
           
        engine.GearBoxAccelerateArray[i] = engine.AccelerateArray[i] * AccelerateArrayFactor / 100;
    }
}
*/

void GearBoxInit()
{

    GearBoxSpeedReset();
    //获取档位配置
    //MotorSpeed.GearBox.MaxGearLevel = DEFAULT_GEAR_LEVEL;
    MotorSpeed.GearBox.GeraLevel = 1;//初始时档位在1档
    //设定第二高速为换挡速度
    
    

    //初始化变速箱齿比
    //
    MotorSpeed.GearBox.Ratio[0] = ((PERCENT_BASE * 200 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[1] = ((PERCENT_BASE * 150 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[2] = ((PERCENT_BASE * 50 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[3] = ((PERCENT_BASE * 25 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[4] = ((PERCENT_BASE * 12 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[5] = ((PERCENT_BASE * 8 * 4) / 10 / 100);
    MotorSpeed.GearBox.Ratio[6] = ((PERCENT_BASE * 6 * 4) / 10 / 100);

    //计算出用于换挡的油门PWM值
    GearBoxThrottlePositionReset();

    MotorSpeed.GearBox.GearBoxStatus = NORMAL_DRIVE;
    //MotorSpeed.GearBox.UpGearDelayTime = 4000;
    //MotorSpeed.GearBox.DownGearDelayTime = 1000;
    MotorSpeed.GearBox.ThrottlePosition = DOWN_GEAR_THROTTLE_POSITION;

    //MotorSpeed.GearBox.UpGearKpDown = 2;

}


uint16_t CalDistSpeed()
{
    uint16_t DistSpeed;
    Turn.TurnVoltage = GetTurnVoltage();
    
    /*if(MotorSpeed.GearBox.GearBoxEnableFlag == DISABLE)
    {
         if(MotorSpeed.SpeedMode == SPEED_MODE_HAL)
         {

             if(MotorSpeed.EvSpeedLevel == EV_SPEED_HOLD)
             {
                 MotorSpeed.DistSpeedRaw = GetDistSpeedWithoutGearBox(Turn.TurnVoltage);
             }
             else
             {
                 MotorSpeed.DistSpeedRaw = GetDistSpeedHal();
             }

             //MotorSpeed.DistSpeedRaw = GetDistSpeedHal();
         }
         else
         {
             //SPEED_MODE_TURN
             MotorSpeed.DistSpeedRaw = GetDistSpeedWithoutGearBox(Turn.TurnVoltage);
         }
    }
    else
    {
       MotorSpeed.DistSpeedRaw = GetDistSpeedWithGearBox(Turn.TurnVoltage);
    }*/
    if(MotorSpeed.EvSpeedLevel == EV_SPEED_HOLD)
    {
        MotorSpeed.DistSpeedRaw = GetDistSpeedWithoutGearBox(Turn.TurnVoltage);
    }
    else
    {
        MotorSpeed.DistSpeedRaw = GetDistSpeedHal();
    }
    //MotorSpeed.DistSpeedRaw = GetDistSpeedHal();
    //刹车检查后的目标速度,若在刹车状态，则强制目标速度为0
    DistSpeed = MotorSpeed.DistSpeedRaw;
    return DistSpeed;
}

bool IsTurnOverStart()
{
    return Turn.TurnVoltage > VOLTAGE_TURN_START? true : false;
}

bool IsTurnBelowStop()
{
    return Turn.TurnVoltage < VOLTAGE_TURN_STOP? true : false;
}






