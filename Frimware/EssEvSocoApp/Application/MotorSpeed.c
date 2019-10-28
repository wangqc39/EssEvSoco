#include "common.h"
#include "MotorSpeed.h"
#include "Engine.h"
#include "ActionTick.h"


#include "MotorSpeedHal.h"





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







#define EV_HOLD_BUTTON			GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9)
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
};


struct TurnInfo Turn;



//#ifndef PROTOCAL_DD
#define ADC1_DR_Address    ((u32)0x4001244C) 
void EvMotorSpeedHwInit()
{
    //三个控制管脚上拉输入
    GPIO_InitTypeDef GPIO_InitStructure;
    DMA_InitTypeDef DMA_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    
    DMA_DeInit(DMA1_Channel1);
    DMA_StructInit(&DMA_InitStructure);
    DMA_InitStructure.DMA_PeripheralBaseAddr = ADC1_DR_Address;
    DMA_InitStructure.DMA_MemoryBaseAddr = (u32)&Turn.TurnAdcValue;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = 1;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Disable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    DMA_Cmd(DMA1_Channel1, ENABLE);


    ADC_StructInit(&ADC_InitStructure);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);
  
    /* ADC1 regular channel14 configuration */ 
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_239Cycles5);
  
    /* Enable ADC1 DMA */
    ADC_DMACmd(ADC1, ENABLE);
    
    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);
  
    /* Enable ADC1 reset calibaration register */   
    ADC_ResetCalibration(ADC1);
    /* Check the end of ADC1 reset calibration register */
    while(ADC_GetResetCalibrationStatus(ADC1));
  
    /* Start ADC1 calibaration */
    ADC_StartCalibration(ADC1);
    /* Check the end of ADC1 calibration */
    while(ADC_GetCalibrationStatus(ADC1));
       
    /* Start ADC1 Software Conversion */ 
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);




    
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

/*
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
*/
}


#define TURN_UP_RESISTANCE		3000//1000
//#define TURN_UP_RESISTANCE		1000

#define TURN_DOWN_RESISTANCE	1000
#define ADC_STANDARD_VOLTAGE			3300
uint16_t GetTurnVoltage()
{
    u32 Temp;
    u32 CurrentVoltage;
    static u32 LastVoltage = 0;
    //计算电压值，精确到小数点后3位									  e
    Temp = ((u32)Turn.TurnAdcValue * (TURN_DOWN_RESISTANCE + TURN_UP_RESISTANCE)) / TURN_DOWN_RESISTANCE;
    CurrentVoltage = (Temp * ADC_STANDARD_VOLTAGE) >> 12;



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

    if(EV_HOLD_BUTTON == Bit_SET)
    {
        //移除了档位的线
        ThisEvSpeedLevel = EV_SPEED_HOLD;
    }
    else
    {
           
        ThisEvSpeedLevel = EV_SPEED3;
    }
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
        MotorSpeed.EvMaxSpeed = (s32)engine.ForwardHighSpeed * EV_SPEED1_SPEED_PERCENT / 100;
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
        MotorSpeed.EvMaxSpeed = (s32)engine.ForwardHighSpeed * EV_SPEED2_SPEED_PERCENT / 100;
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
    MotorSpeed.EvHoldSpeed = engine.ForwardLowSpeed + ((s32)((s32)MotorSpeed.EvMaxSpeed - engine.ForwardLowSpeed) * EV_HOLD_SPEED) / 100; 
}

//#endif




//#ifndef PROTOCAL_DD
/******************绝对目标速度计算模块*************************/
s16 GestEvDistSpeed(s16 DistSpeedRaw)
{
    static bool SpeedOverHoldFlag = FALSE;//之前速度是否超过AFTER_MAX_SPEED_ENGINE_DOWN_PERCENT的标志
    static uint32_t OverHoldTime = 0;
    static uint32_t LastHoldSpeedFlushTime;//在HOLD速度干预过程中，速度更新的时刻
    static s16 LastHoldSpeed;//速度在HOLD区间内超过指定时间后，对速度进行干预时的当前速度
    
    s16 DistSpeed;

    if(SpeedOverHoldFlag == FALSE)
    {
        //之前速度在HOLD之下
        if(DistSpeedRaw >= MotorSpeed.EvHoldSpeed && MotorSpeed.RealSpeed >= DistSpeedRaw)
        {
            //速度超过HOLD，记录进入HOLD区域的时刻
            OverHoldTime = GetSystemTime();
            SpeedOverHoldFlag = TRUE;
        }
        DistSpeed = DistSpeedRaw;
    }
    else
    {
        if(DistSpeedRaw < MotorSpeed.EvHoldSpeed)
        {
            //速度回落到HOLD以下，不进行速度干预
            DistSpeed = DistSpeedRaw;
            SpeedOverHoldFlag = FALSE;
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


int16_t GetDistSpeedWithoutGearBox(uint16_t TurnVoltage)
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
                PwmDifference = (s32)((s32)MotorSpeed.HighCycle - MotorSpeed.GearBox.ChangeGearThrottlePosition);
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
s16 DoAccelerateWithMotorCurve(s16 NowSpeed, s16 Error, u32 GearRatio)
{
    s16 DistError;
    int i;
    s32 Accelerate;
    //u16 AccelerateLow;
    //u16 AccelerateHigh;
    s32 percent;
    s32 temp;
    s32 ErrorFactor;

    

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
                Accelerate = (s32)engine.AccelerateArrayOri[0] * GearRatio;
                Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
            }
            else if(i >= engine.ForwardAudioFileNum)
            {
                //若发动机速度大于发动机最大速度，则直接采用发动机最大速度时的力矩
                Accelerate = (s32)engine.AccelerateArrayOri[engine.ForwardAudioFileNum - 1] * GearRatio;
                Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
            }
            else
            {
                //中间阶段，通过百分比来计算力矩的值
                if(engine.AccelerateArrayOri[i] == engine.AccelerateArrayOri[i - 1])
                {
                    //若相邻的两个加速度力矩相同，则无需计算
                    Accelerate = (s32)engine.AccelerateArrayOri[i] * GearRatio;
                    Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
                }
                else
                {
                    //根据百分比计算当下的加速度力矩
                    percent = (s32)((s32)(NowSpeed - engine.ForwardSpeedArray[i - 1]) * PERCENT_BASE);
                    percent = percent / (engine.ForwardSpeedArray[i] - engine.ForwardSpeedArray[i - 1]);
                    if(engine.AccelerateArrayOri[i] > engine.AccelerateArrayOri[i - 1])
                    {
                        Accelerate = (s32)((s32)(engine.AccelerateArrayOri[i] - engine.AccelerateArrayOri[i - 1]) * percent);
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i - 1];
                    }
                    else
                    {
                        Accelerate = (s32)((s32)(engine.AccelerateArrayOri[i - 1] - engine.AccelerateArrayOri[i]) * (PERCENT_BASE - percent)); 
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i];                       
                    }
                    Accelerate = Accelerate * GearRatio;
                    Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
                }
    
            }
    
            //根据加速度的力矩和速度差值(油门大小)来计算增加的速度
            percent = (s32)(Error << PERCENT_BASE_SHIFT) / engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 1];
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
                DistError = (s16)((Error * KP_SPEED_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
            }
            else
            {
                DistError = (s16)((Error * KP_UPGEAR_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
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
                    percent = (s32)((s32)(engine.BackwardSpeedArray[i - 1] - NowSpeed) * PERCENT_BASE);
                    percent = percent / (engine.BackwardSpeedArray[i - 1] - engine.BackwardSpeedArray[i]);
                    if(engine.AccelerateArrayOri[i] > engine.AccelerateArrayOri[i - 1])
                    {
                        Accelerate = (s32)((s32)(engine.AccelerateArrayOri[i] - engine.AccelerateArrayOri[i - 1]) * percent);
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i - 1];
                    }
                    else
                    {
                        Accelerate = (s32)((s32)(engine.AccelerateArrayOri[i - 1] - engine.AccelerateArrayOri[i]) * (PERCENT_BASE - percent)); 
                        Accelerate = (Accelerate >> PERCENT_BASE_SHIFT) + engine.AccelerateArrayOri[i];
                    }
                    Accelerate = Accelerate * GearRatio;
                    Accelerate = Accelerate >> PERCENT_BASE_SHIFT;
                }
    
            }
    
            //根据加速度的力矩和速度差值(油门大小)来计算增加的速度
            percent = (s32)(Error << PERCENT_BASE_SHIFT) / engine.BackwardSpeedArray[engine.BackwardAudioFileNum - 1];
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
                DistError = (s16)((Error * KP_SPEED_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
            }
            else
            {
                DistError = (s16)((Error * KP_UPGEAR_DOWN_PERCENT / 100));// >> REAL_SPEED_CAL_MULT_SHIFT);
            }
            
        }
    }
    return DistError;
}

//通过强制控制RealSpeed来控制是否播放过载声音
s16 EngineOverloadHandler(s16 RealSpeed)
{
    static u32 InOverloadSpeedTime = 0;

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

s16 RealSpeedOtherFilter(s16 RealSpeed)
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



s16 GetMotorRealSpeedWithoutGearbox(s16 DistSpeed, s16 RealSpeed)
{
    s16 Error;
    s16 Adjust;
    
    //static s16 RealSpeed = 0;
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
        Adjust = DoAccelerateWithMotorCurve(RealSpeed, Error, (u32)(MotorSpeed.EvAccRatio >> REAL_SPEED_CAL_MULT_SHIFT));
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


s16 GetMotorRealSpeedWithGearBox(s16 DistSpeed, s16 RealSpeed)
{
    s16 Error;
    s16 Adjust;
    uint16_t EngineAcc;
    
    //static s16 RealSpeed = 0;

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
        MotorSpeed.GearBox.ChangeGearSpeed = (s32)engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 2] * MotorSpeed.GearBox.UpGearSpeedPercent / 100;
    }
    else
    {
        MotorSpeed.GearBox.ChangeGearSpeed = (s32)engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 1] * MotorSpeed.GearBox.UpGearSpeedPercent / 100;
    }

    //设定换挡完成后发动机应用的速度
    MotorSpeed.GearBox.AfterUpGearSpeed = (s32)MotorSpeed.GearBox.ChangeGearSpeed * MotorSpeed.GearBox.AfterUpGearSpeedPercent / 100;
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
       MotorSpeed.GearBox.UpGearThrottlePosition = (u32)(RcCapture.ForwardHighPwm - RcCapture.MiddleThrottlePwm) 
                                                                                     * UP_GEAR_THROTTLE_PERCENT / 100 + RcCapture.MiddleThrottlePwm;
       MotorSpeed.GearBox.DownGearThrottlePosition = (u32)(RcCapture.ForwardHighPwm - RcCapture.MiddleThrottlePwm) 
                                                                                     * DOWN_GEAR_THROTTLE_PERCENT / 100 + RcCapture.MiddleThrottlePwm;                                                                              
    }
    else
    {
        MotorSpeed.GearBox.UpGearThrottlePosition = (u32)(RcCapture.MiddleThrottlePwm - RcCapture.ForwardHighPwm) 
                                                                                     * (100 - UP_GEAR_THROTTLE_PERCENT) / 100 + RcCapture.ForwardHighPwm;
        MotorSpeed.GearBox.DownGearThrottlePosition = (u32)(RcCapture.MiddleThrottlePwm - RcCapture.ForwardHighPwm) 
                                                                                     * (100 - DOWN_GEAR_THROTTLE_PERCENT) / 100 + RcCapture.ForwardHighPwm;         
    }
*/
    MotorSpeed.GearBox.UpGearThrottlePosition = ((u32)(VOLTAGE_TURN_MAX - VOLTAGE_TURN_IDLE) * UP_GEAR_THROTTLE_PERCENT) / 100 + VOLTAGE_TURN_IDLE;
    MotorSpeed.GearBox.DownGearThrottlePosition = ((u32)(VOLTAGE_TURN_MAX - VOLTAGE_TURN_IDLE) * DOWN_GEAR_THROTTLE_PERCENT) / 100 + VOLTAGE_TURN_IDLE; 
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
    return Turn.TurnVoltage > VOLTAGE_TURN_START? TRUE : FALSE;
}

bool IsTurnBelowStop()
{
    return Turn.TurnVoltage < VOLTAGE_TURN_STOP? TRUE : FALSE;
}






