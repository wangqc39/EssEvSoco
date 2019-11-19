#include "common.h"
#include "mixer.h"
#include "engine.h"
#include "fs.h"
#include "OnChipAudio.h"
#include "SystemConfig.h"
#include "MixerConfig.h"
#include "SystemConfig.h"
#include "OnChipAudio.h"
#include "ActionTick.h"
#include "SystemError.h"
#include "DownLoader.h"
#include "SystemInfo.h"

#include "MotorSpeed.h"




#define MIXER_OUT_TIMER				TIM1

//SDZ控制
#define AMPLIFIER_ENABLE				GPIO_SetBits(GPIOA, GPIO_Pin_6);
#define AMPLIFIER_DISABLE			GPIO_ResetBits(GPIOA, GPIO_Pin_6);





void AmplifierDisable(void);
unsigned char GetSystemVolume(void);

uint8_t RealVolume;

struct MixerInfo mixer = 
{
.MixerEnableFlag = ENABLE,
.VolumeSlopePercent = 10000
};


/**************************初始化模块*******************************/
void AudioOutHwConfig()
{
    NVIC_InitTypeDef 			NVIC_InitStructure;
    TIM_TimeBaseInitTypeDef    	AudioPlayTimeBaseStructure;
    DAC_InitTypeDef            		AudioPlayDACStructure;
    GPIO_InitTypeDef 			GPIO_InitStructure;
    
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    
    //DAC output
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    //功放管脚输出
    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    AmplifierDisable();
    
   
    TIM_TimeBaseStructInit(&AudioPlayTimeBaseStructure); 
    AudioPlayTimeBaseStructure.TIM_Period = DAC_OUT_ARR_VALUE;          
    AudioPlayTimeBaseStructure.TIM_Prescaler = 0;       
    AudioPlayTimeBaseStructure.TIM_ClockDivision = 0x0;    
    AudioPlayTimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  
    TIM_TimeBaseInit(MIXER_OUT_TIMER, &AudioPlayTimeBaseStructure);
    TIM_ITConfig(MIXER_OUT_TIMER, TIM_IT_Update, ENABLE);
    TIM_Cmd(MIXER_OUT_TIMER, ENABLE);
    
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQChannel;//TIM8_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    DAC_StructInit(&AudioPlayDACStructure);
    AudioPlayDACStructure.DAC_Trigger = DAC_Trigger_Software;
    AudioPlayDACStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
    AudioPlayDACStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
    DAC_Init(DAC_Channel_2, &AudioPlayDACStructure);  	
    DAC_Cmd(DAC_Channel_2, ENABLE);
}
 




/************************音频输出控制模块*****************************/
void StopMixer()
{
    MixerChannel[0].SoundData = 0;
    MixerChannel[1].SoundData = 0;
    EngineStartStopVoiceChannel.SoundData = 0;
}

/*void StartMixer()
{
    TIM_Cmd(MIXER_CHANNEL0_TIMER, ENABLE);
    TIM_Cmd(MIXER_CHANNEL1_TIMER, ENABLE);
    TIM_Cmd(MIXER_OUT_TIMER, ENABLE);
}*/


/*****************************读取声音数据**************************************/
void MixerChaneBuff0Read(struct MixerChannelInfo *ThisChannel)
{
    unsigned char IdType;
    ReadFile(ThisChannel->Fp, ThisChannel->AudioStartAddress + ThisChannel->ReadFileCnt, (u8 *)ThisChannel->buff0, AUDIO_DATA_READ_CNT);
    ThisChannel->ReadFileCnt += AUDIO_DATA_READ_CNT;
    //实际语音数据量
    //ThisChannel->Buff0DataCnt = AUDIO_DATA_READ_CNT / 3 * 2;
    ThisChannel->Buff0DataCnt = AUDIO_DATA_READ_CNT >> 1;
    //BuffReadFlag[1] = MixerChannel[1].DataPtr;
    //BuffReadFlag[2] = MixerChannel[2].DataPtr;
    if(ThisChannel->ReadFileCnt >= ThisChannel->TotalDataCnt)
    {
        //最后一次读取，读取的数据比需要的数据多，则减去多余部分
        //ThisChannel->Buff0DataCnt -= (ThisChannel->ReadFileCnt - ThisChannel->TotalDataCnt) / 3 * 2;
        ThisChannel->Buff0DataCnt -= (ThisChannel->ReadFileCnt - ThisChannel->TotalDataCnt) >> 1;
        ThisChannel->ReadFileCnt = 0;
        IdType = ThisChannel->id & 0xF0;
        if(IdType == 0x20)
        {
            //泄压阀一次播放完成处理
            if(ThisChannel->PlayCnt != 0xFF)
            {
                ThisChannel->PlayCnt--;
                if(ThisChannel->PlayCnt == 0)
                {
                    ThisChannel->AmplitudeFactor = 0;
                    ThisChannel->SoundData = 0;
                }
            }
        }
        else if(IdType == 0x10)
        {
            //SpecialPlayOnceFinishHandler();
        }
        ThisChannel->CycleCnt++;
    }
    ThisChannel->Buff0ReadyFlag = 1;
}

void MixerChaneBuff1Read(struct MixerChannelInfo *ThisChannel)
{
    unsigned char IdType;
    ReadFile(ThisChannel->Fp, ThisChannel->AudioStartAddress + ThisChannel->ReadFileCnt, (u8 *)ThisChannel->buff1, AUDIO_DATA_READ_CNT);
    ThisChannel->ReadFileCnt += AUDIO_DATA_READ_CNT;
    //实际语音数据量
    //ThisChannel->Buff1DataCnt = AUDIO_DATA_READ_CNT / 3 * 2;
    ThisChannel->Buff1DataCnt = AUDIO_DATA_READ_CNT >> 1;
    if(ThisChannel->ReadFileCnt >= ThisChannel->TotalDataCnt)
    {
        //最后一次读取，读取的数据比需要的数据多，则减去多余部分
        //ThisChannel->Buff1DataCnt -= (ThisChannel->ReadFileCnt - ThisChannel->TotalDataCnt) / 3 * 2;
        ThisChannel->Buff1DataCnt -= (ThisChannel->ReadFileCnt - ThisChannel->TotalDataCnt) >> 1;
        ThisChannel->ReadFileCnt = 0;
        IdType = ThisChannel->id & 0xF0;
        if(IdType == 0x20)
        {
            //泄压阀一次播放完成处理
            if(ThisChannel->PlayCnt != 0xFF)
            {
                ThisChannel->PlayCnt--;
                if(ThisChannel->PlayCnt == 0)
                {
                    ThisChannel->AmplitudeFactor = 0;
                    ThisChannel->SoundData = 0;
                }
            }
        }
        else if(IdType == 0x10)
        {
            //SpecialPlayOnceFinishHandler();
        }
        ThisChannel->CycleCnt++;
    }
    ThisChannel->Buff1ReadyFlag = 1;
}

void MixerChaneBuff2Read(struct MixerChannelInfo *ThisChannel)
{
    unsigned char IdType;
    ReadFile(ThisChannel->Fp, ThisChannel->AudioStartAddress + ThisChannel->ReadFileCnt, (u8 *)ThisChannel->buff2, AUDIO_DATA_READ_CNT);
    ThisChannel->ReadFileCnt += AUDIO_DATA_READ_CNT;
    //实际语音数据量
    //ThisChannel->Buff2DataCnt = AUDIO_DATA_READ_CNT / 3 * 2;
    ThisChannel->Buff2DataCnt = AUDIO_DATA_READ_CNT >> 1;
    if(ThisChannel->ReadFileCnt >= ThisChannel->TotalDataCnt)
    {
        //最后一次读取，读取的数据比需要的数据多，则减去多余部分
        //ThisChannel->Buff2DataCnt -= (ThisChannel->ReadFileCnt - ThisChannel->TotalDataCnt) / 3 * 2;
        ThisChannel->Buff2DataCnt -= (ThisChannel->ReadFileCnt - ThisChannel->TotalDataCnt) >> 1;
        ThisChannel->ReadFileCnt = 0;
        IdType = ThisChannel->id & 0xF0;
        if(IdType == 0x20)
        {
            //泄压阀一次播放完成处理
            if(ThisChannel->PlayCnt != 0xFF)
            {
                ThisChannel->PlayCnt--;
                if(ThisChannel->PlayCnt == 0)
                {
                    ThisChannel->AmplitudeFactor = 0;
                    ThisChannel->SoundData = 0;
                }
            }
        }
        else if(IdType == 0x10)
        {
            //SpecialPlayOnceFinishHandler();
        }
        ThisChannel->CycleCnt++;
    }
    ThisChannel->Buff2ReadyFlag = 1;            
}


void AudioFileReadHandler(struct MixerChannelInfo *ThisChannel)
{
    
    //unsigned int read;
    if(ThisChannel->NowBuffFlag == 0)
    {
        if(ThisChannel->Buff0ReadyFlag == 0)
        {
            MixerChaneBuff0Read(ThisChannel);
        }
        else if(ThisChannel->Buff1ReadyFlag == 0)
        {
            MixerChaneBuff1Read(ThisChannel);
        }
        else if(ThisChannel->Buff2ReadyFlag == 0)
        {
            MixerChaneBuff2Read(ThisChannel);
        }
    }
    else if(ThisChannel->NowBuffFlag == 1)
    {
        if(ThisChannel->Buff1ReadyFlag == 0)
        {
            MixerChaneBuff1Read(ThisChannel);
        }
        else if(ThisChannel->Buff2ReadyFlag == 0)
        {
            MixerChaneBuff2Read(ThisChannel);
        }
        else if(ThisChannel->Buff0ReadyFlag == 0)
        {
            MixerChaneBuff0Read(ThisChannel);
        }
    }
    else if(ThisChannel->NowBuffFlag == 2)
    {
        if(ThisChannel->Buff2ReadyFlag == 0)
        {
            MixerChaneBuff2Read(ThisChannel);
        }
        else if(ThisChannel->Buff0ReadyFlag == 0)
        {
            MixerChaneBuff0Read(ThisChannel);
        }
        else if(ThisChannel->Buff1ReadyFlag == 0)
        {
            MixerChaneBuff1Read(ThisChannel);
        }
    }

    

}


/***********************准备声音数据***********************************/
void AudioChannelGetDataInTimeInterruptWithSecure(struct MixerChannelInfo *ThisChannel)
{
    s32 DataTemp;
    s16 RealData;
    u16 DataPosition;
    if(ThisChannel->AmplitudeFactor != 0)
    {
        if(ThisChannel->NowBuffFlag == 0 && ThisChannel->Buff0ReadyFlag != 0)
        {
            //对语音数据进行解密
            DataPosition = ThisChannel->DataPtr * 2;
            RealData = (s16)(((ThisChannel->buff0[DataPosition + 1] ^ 0xAA) << 8) | (u8)((ThisChannel->buff0[DataPosition] + 0xD7)));
            DataTemp = ((s32)RealData * (s32)ThisChannel->AmplitudeFactor) >> PERCENT_MIXER_BASE_SHIFT;
            ThisChannel->SoundData = DataTemp;
            ThisChannel->DataPtr++;
            if(ThisChannel->DataPtr >= ThisChannel->Buff0DataCnt)
            {
                ThisChannel->DataPtr = 0;
                ThisChannel->Buff0ReadyFlag = 0;
                if(ThisChannel->Buff1ReadyFlag == 1)
                {
                    ThisChannel->NowBuffFlag = 1;
                }
                else
                {
                    //ThisChannel->OverFlow0++;
                }
            }
        }
        else if(ThisChannel->NowBuffFlag == 1 && ThisChannel->Buff1ReadyFlag != 0)
        {
            DataPosition = ThisChannel->DataPtr * 2;
            RealData = (s16)(((ThisChannel->buff1[DataPosition + 1] ^ 0xAA) << 8) | (u8)((ThisChannel->buff1[DataPosition] + 0xD7)));
            DataTemp = ((s32)RealData * (s32)ThisChannel->AmplitudeFactor) >> PERCENT_MIXER_BASE_SHIFT;
            ThisChannel->SoundData = DataTemp;
            ThisChannel->DataPtr++;
            if(ThisChannel->DataPtr >= ThisChannel->Buff1DataCnt)
            {
                ThisChannel->DataPtr = 0;
                ThisChannel->Buff1ReadyFlag = 0;
                if(ThisChannel->Buff2ReadyFlag == 1)
                {
                    ThisChannel->NowBuffFlag = 2;
                }
                else
                {
                    //ThisChannel->OverFlow1++;
                }
            }
        }
        else if(ThisChannel->NowBuffFlag == 2 && ThisChannel->Buff2ReadyFlag != 0)
        {
            DataPosition = ThisChannel->DataPtr * 2;
            RealData = (s16)(((ThisChannel->buff2[DataPosition + 1] ^ 0xAA) << 8) | (u8)((ThisChannel->buff2[DataPosition] + 0xD7)));
            DataTemp = ((s32)RealData * (s32)ThisChannel->AmplitudeFactor) >> PERCENT_MIXER_BASE_SHIFT;
            ThisChannel->SoundData = DataTemp;
            ThisChannel->DataPtr++;
            if(ThisChannel->DataPtr >= ThisChannel->Buff2DataCnt)
            {
                ThisChannel->DataPtr = 0;
                ThisChannel->Buff2ReadyFlag = 0;
                if(ThisChannel->Buff0ReadyFlag == 1)
                {
                    ThisChannel->NowBuffFlag = 0;
                }
                else
                {
                    //ThisChannel->OverFlow2++;
                }
            }
            
        }
        else
        {
            ThisChannel->SoundData = 0;
        }
    
    }
    else
    {
        ThisChannel->SoundData = 0;
    }
}
//void AudioChannelGetDataInTimeInterruptWithSecure(struct MixerChannelInfo *ThisChannel)
//{
//    s32 DataTemp;
//    s16 RealData;
//    u16 DataPosition;
//    if(ThisChannel->AmplitudeFactor != 0)
//    {
//        if(ThisChannel->NowBuffFlag == 0 && ThisChannel->Buff0ReadyFlag != 0)
//        {
//            //对语音数据进行解密
//            DataPosition = (ThisChannel->DataPtr / 2) * 3;
//            if(ThisChannel->DataPtr % 2 == 0)
//            {
//                RealData = (s16)(((ThisChannel->buff0[DataPosition] - 0x31) << 8) | ((ThisChannel->buff0[DataPosition + 2] - 0x75) & 0xF0));
//            }
//            else
//            {
//                RealData = (s16)(((ThisChannel->buff0[DataPosition + 1] ^ 0x55) << 8) | (((ThisChannel->buff0[DataPosition + 2] - 0x75) << 4) & 0xF0));
//            }
//            
//            DataTemp = ((s32)RealData * (s32)ThisChannel->AmplitudeFactor) >> PERCENT_MIXER_BASE_SHIFT;
//            ThisChannel->SoundData = DataTemp;
//            ThisChannel->DataPtr++;
//            if(ThisChannel->DataPtr >= ThisChannel->Buff0DataCnt)
//            {
//                ThisChannel->DataPtr = 0;
//                ThisChannel->Buff0ReadyFlag = 0;
//                if(ThisChannel->Buff1ReadyFlag == 1)
//                {
//                    ThisChannel->NowBuffFlag = 1;
//                }
//                else
//                {
//                    //ThisChannel->OverFlow0++;
//                }
//            }
//        }
//        else if(ThisChannel->NowBuffFlag == 1 && ThisChannel->Buff1ReadyFlag != 0)
//        {
//            DataPosition = (ThisChannel->DataPtr / 2) * 3;

//            if(ThisChannel->DataPtr % 2 == 0)
//            {
//                RealData = (s16)(((ThisChannel->buff1[DataPosition] - 0x31) << 8) | ((ThisChannel->buff1[DataPosition + 2] - 0x75) & 0xF0));
//            }
//            else
//            {
//                RealData = (s16)(((ThisChannel->buff1[DataPosition + 1] ^ 0x55) << 8) | (((ThisChannel->buff1[DataPosition + 2] - 0x75) << 4) & 0xF0));
//            }
//            DataTemp = ((s32)RealData * (s32)ThisChannel->AmplitudeFactor) >> PERCENT_MIXER_BASE_SHIFT;
//            ThisChannel->SoundData = DataTemp;
//            ThisChannel->DataPtr++;
//            if(ThisChannel->DataPtr >= ThisChannel->Buff1DataCnt)
//            {
//                ThisChannel->DataPtr = 0;
//                ThisChannel->Buff1ReadyFlag = 0;
//                if(ThisChannel->Buff2ReadyFlag == 1)
//                {
//                    ThisChannel->NowBuffFlag = 2;
//                }
//                else
//                {
//                    //ThisChannel->OverFlow1++;
//                }
//            }
//        }
//        else if(ThisChannel->NowBuffFlag == 2 && ThisChannel->Buff2ReadyFlag != 0)
//        {
//            DataPosition = (ThisChannel->DataPtr / 2) * 3;
//            if(ThisChannel->DataPtr % 2 == 0)
//            {
//                RealData = (s16)(((ThisChannel->buff2[DataPosition] - 0x31) << 8) | ((ThisChannel->buff2[DataPosition + 2] - 0x75) & 0xF0));
//            }
//            else
//            {
//                RealData = (s16)(((ThisChannel->buff2[DataPosition + 1] ^ 0x55) << 8) | (((ThisChannel->buff2[DataPosition + 2] - 0x75) << 4) & 0xF0));
//            }
//            DataTemp = ((s32)RealData * (s32)ThisChannel->AmplitudeFactor) >> PERCENT_MIXER_BASE_SHIFT;
//            ThisChannel->SoundData = DataTemp;
//            ThisChannel->DataPtr++;
//            if(ThisChannel->DataPtr >= ThisChannel->Buff2DataCnt)
//            {
//                ThisChannel->DataPtr = 0;
//                ThisChannel->Buff2ReadyFlag = 0;
//                if(ThisChannel->Buff0ReadyFlag == 1)
//                {
//                    ThisChannel->NowBuffFlag = 0;
//                }
//                else
//                {
//                    //ThisChannel->OverFlow2++;
//                }
//            }
//        }
//        else
//        {
//            ThisChannel->SoundData = 0;
//        }
//    
//    }
//}



/*********************声音输出中断处理***************************/
void AudioOutDac()
{
    u16 DacDataL;
    s32 data;
    s32 EngineData;
    s32 SystemAudioData;

    if(mixer.Slinet == false)
    {
        EngineData = GetEngineAudioData();
    }
    else
    {
        EngineData = 0;
    }
    
    SystemAudioData = GetCurrentOnChipAudioData();
    data = EngineData + SystemAudioData;

    
    //音量控制
    data = (s32)data * RealVolume;//GetSystemVolume());
    data = data >> 6;

    if(data > 32767)
    {
        data = 32767;
    }
    else if(data < -32768)
    {
        data = -32768;
    }

    DacDataL = data + 0x8000;
    DAC_SetChannel2Data(DAC_Align_12b_L,  DacDataL); 

    DAC->SWTRIGR = 0x02;   


}


void AudioOutTimerIntHandler()
{
    u16 DacDataL;
    s32 data;
    s32 EngineData;
    s32 SystemAudioData;
    TIM_ClearITPendingBit(MIXER_OUT_TIMER, TIM_IT_Update);
    
    

    if(mixer.Slinet == false)
    {
        EngineData = GetEngineAudioData();
    }
    else
    {
        EngineData = 0;
    }
    
    SystemAudioData = GetOnChipAudioData();
    data = EngineData + SystemAudioData;

    
    //音量控制
    data = (s32)data * RealVolume;//GetSystemVolume());
    data = data >> 6;

    if(data > 32767)
    {
        data = 32767;
    }
    else if(data < -32768)
    {
        data = -32768;
    }

    DacDataL = data + 0x8000;
    DAC_SetChannel2Data(DAC_Align_12b_L,  DacDataL); 

    DAC->SWTRIGR = 0x02;   

}




/************************声音切换**************************/
//unsigned char TurnToAnotherIndex(unsigned char Index)
//{
//    if(Index == 0)
//    {
//        return 1;
//    }
//    else
//    {
//        return 0;
//    }
//}


//0:该声音索引下有可用声音
//-1:该声音索引下没有可用声音
int AnalyOneSoundIndex(unsigned char EngineIndex)
{
    int EngineRet;//, SpecialRet;

    AnalyzeMixerConfig(EngineIndex);
    EngineRet = AnalyzeEngineFile(EngineIndex);
//    AnalyzeBreakFile(EngineIndex);
//    AnalyzeTurboFile(EngineIndex);
//    SpecialRet = AnalyzeAuxFile(EngineIndex);
    

    if(EngineRet == 0)// || SpecialRet == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

//尝试切换声音，如果DistIndex的声音不存在，则向后遍历所有Vehicle，尝试找到一个可用的Vehicle
int32_t TryChangeVehileIndexLoop(unsigned char DistIndex, bool IsStartUp)
{
    int32_t i;
    int ret;
    int result;
    unsigned char VolumeLevelBk;
    uint8_t index;
    uint8_t SelectIndex;
    ret = AnalyOneSoundIndex(DistIndex);
    if(ret == 0)
    {
        //索引成功，直接返回
        //无需修改配置表
        SelectIndex = DistIndex;
        result = 0;
        goto Finish;
    }

    //尝试继续向后遍历声音，尝试找到一个可用的声音
    for(i = 0; i < VEHICLE_CNT - 1; i++)
    {
        index = (DistIndex + 1 + i) % VEHICLE_CNT;
        ret = AnalyOneSoundIndex(index);
        if(ret == 0)
        {
            SelectIndex = index;
            result = 1;
            goto Finish;
        }
    }

    //无可用的引擎声音，mixer.SoundIndex 不变
    result = 2;
    SelectIndex = DistIndex;
Finish:
    if(mixer.SoundIndex != SelectIndex)
    {
        //当mixer.SoundIndex变化时，进行保存
        mixer.SoundIndex = SelectIndex;
        WriteSystemParamTable();
    }
    if(IsStartUp == FALSE)
    {
        VolumeLevelBk = mixer.VolumeLevel;
        mixer.VolumeLevel = 2;
        RealVolume = 16;
        //使能功放设置
        AmplifierEnable();
        mDelay(500);
        OnChipAudioPlay(2, mixer.SoundIndex + 1, 100);
        mDelay(1000);
    
        mixer.VolumeLevel = VolumeLevelBk;
        RealVolume = mixer.VolumeLevel * 8;
    }
    return result;

}

//尝试切换声音，如果DistIndex的声音不存在，则回滚到之前选择的声音
int32_t TryChangeVehileRollBack(unsigned char DistIndex, bool IsStartUp)
{
    int ret;
    int result;
    unsigned char VolumeLevelBk;
    uint8_t SelectIndex;
    ret = AnalyOneSoundIndex(DistIndex);
    if(ret == 0)
    {
        //索引成功，直接返回
        //无需修改配置表
        SelectIndex = DistIndex;
        result = 0;
        goto Finish;
    }

    //返回最初的声音
    ret = AnalyOneSoundIndex(mixer.SoundIndex);
    {
        if(ret == 0)
        {
            //声音返回成功，
            SelectIndex = mixer.SoundIndex;
            result = 1;
            goto Finish;
        }
    }

    SelectIndex = mixer.SoundIndex;
    result = 2;
Finish:
    if(mixer.SoundIndex != SelectIndex)
    {
        //当mixer.SoundIndex变化时，进行保存
        mixer.SoundIndex = SelectIndex;
        WriteSystemParamTable();
    }
    

    if(IsStartUp == FALSE)
    {
        VolumeLevelBk = mixer.VolumeLevel;
        mixer.VolumeLevel = 2;
        RealVolume = 16;
        //使能功放设置
        AmplifierEnable();
        mDelay(500);
        OnChipAudioPlay(2, mixer.SoundIndex + 1, 100);
        mDelay(1000);
    
        mixer.VolumeLevel = VolumeLevelBk;
        RealVolume = mixer.VolumeLevel * 8;
    }
    
    return result;

}


/*
//返回0:切换成功;1:切换失败，指定车辆的引擎声音不存在，切换到一个可用的车辆
// 2: 切换失败，无可用的引擎声音
int TryChangeSoundIndex(unsigned char DistIndex)
{
    int ret;
    int32_t i;
    int result;
    unsigned char VolumeLevelBk;
    uint8_t index;
    if(DistIndex == mixer.SoundIndex)
    {
        //当期望的索引和系统当前配置一样时
        //场景:系统上电或通过RC PLUS进行声音的切换
        //尝试获取系统配置索引号的飞机声音信息
        ret = AnalyOneSoundIndex(DistIndex);
        if(ret == 0)
        {
            //索引成功，直接返回
            //无需修改配置表
            result =  0;
            goto Finish;
        }

        //尝试继续向后遍历声音，尝试找到一个可用的声音
        for(i = 0; i < VEHICLE_CNT - 1; i++)
        {
            index = (DistIndex + 1 + i) % VEHICLE_CNT;
            ret = AnalyOneSoundIndex(index);
            if(ret == 0)
            {
                mixer.SoundIndex = index;
                WriteSystemParamTable();
                result = 1;
                goto Finish;
            }
        }

        //无可用的引擎声音，mixer.SoundIndex 不变
        result = 2;
    }
    else
    {
        //场景声音切换
        ret = AnalyOneSoundIndex(DistIndex);
        if(ret == 0)
        {
            //索引成工，但是和系统配置不一致，写入配置
            mixer.SoundIndex = DistIndex;
            WriteSystemParamTable();
            result = 0;
            goto Finish;
        }

        //返回最初的声音
        ret = AnalyOneSoundIndex(mixer.SoundIndex);
        {
            if(ret == 0)
            {
                //声音返回成功，
                result = 1;
                goto Finish;
            }
        }

         //尝试遍历声音，尝试找到一个可用的声音
        for(i = 0; i < VEHICLE_CNT; i++)
        {
            ret = AnalyOneSoundIndex(i);
            if(ret == 0)
            {
                mixer.SoundIndex = DistIndex;
                WriteSystemParamTable();
                result = 1;
                goto Finish;
            }
        }

        //无可用的引擎声音，mixer.SoundIndex 不变
        result = 2;
    }

Finish:
    VolumeLevelBk = mixer.VolumeLevel;
    mixer.VolumeLevel = 2;
    //使能功放设置
    AmplifierEnable();

    mDelay(500);
    
    OnChipAudioPlay(2, mixer.SoundIndex + 1, 100);

    mDelay(1000);

    mixer.VolumeLevel = VolumeLevelBk;
    return result;
}
*/


/*
void ChangeSoundIndex()
{
    unsigned char DistIndex;
    unsigned char VolumeLevelBk;
    if(mixer.SoundIndex == 0)
    {

        DistIndex = 1;
    }
    else
    {
        DistIndex = 0;
    }

    VolumeLevelBk = mixer.VolumeLevel;
    mixer.VolumeLevel = 2;
    //使能功放设置
    AmplifierEnable();

    mDelay(500);
    
    TryChangeSoundIndex(DistIndex);
    OnChipAudioPlay(2, mixer.SoundIndex + 1, 100);

    mDelay(1000);

    mixer.VolumeLevel = VolumeLevelBk;
}
*/


/************************音量及功放控制************************/
#define MAX_VOLUME			8
//static unsigned char VolumeArray[MAX_VOLUME + 1] = {0, 3, 7, 13, 20, 28, 38, 50, 64};
//static unsigned char VolumeArray[MAX_VOLUME + 1] = {0, 8, 16, 24, 32, 40, 48, 56, 64};

void AddVolume()
{
    //if(mixer.VolumeLevel < MAX_VOLUME)
    //{
    //    mixer.VolumeLevel++;
    //    WriteSystemParamTable();//WriteVolumeConfig(mixer.VolumeLevel);
    //}
}


void MinusVolume()
{

    if(mixer.VolumeLevel > MAX_VOLUME)
    {
        mixer.VolumeLevel = MAX_VOLUME;
    }
    
    if(mixer.VolumeLevel > MIN_VOLUME)
    {
        mixer.VolumeLevel--;
    }
    else
    {
        mixer.VolumeLevel = MAX_VOLUME;
    }

    WriteSystemParamTable();
}

void AmplifierEnable()
{
    mixer.AmplifierEnable = ENABLE;
    AMPLIFIER_ENABLE;
}

void AmplifierDisable()
{
    mixer.AmplifierEnable = DISABLE;
    AMPLIFIER_DISABLE;
}

void AmplifierControlHandler()
{
    if(mixer.AmplifierEnable == DISABLE)
    {
        if((((engine.status != ENGINE_STOP && mixer.Slinet == false && mixer.MixerEnableFlag != DISABLE) || CheckOnChipAudioIsPlaying() != 0) && mixer.VolumeLevel != 0)
            || SystemError.StartBatteryHighError == START_BATTERY_HIGH_ERROR)
        {
            AmplifierEnable();
        }
    }
    else
    {
        if((((engine.status == ENGINE_STOP || mixer.Slinet != false || mixer.MixerEnableFlag == DISABLE)  && CheckOnChipAudioIsPlaying() == 0) || mixer.VolumeLevel == 0)
            && SystemError.StartBatteryHighError != START_BATTERY_HIGH_ERROR)
        {
            AmplifierDisable();
        }
    }
}


int VolumeSetBySystemConfig(unsigned char *VolumeLevel)
{
    if(*VolumeLevel > MAX_VOLUME)
    {
        *VolumeLevel = DEFAULT_SOUND_LEVEL;
        mixer.VolumeLevel = *VolumeLevel;
        return -1;
    }
    mixer.VolumeLevel = *VolumeLevel;
    return 0;
}


/*unsigned char GetSystemVolumeLevel()
{
    return mixer.VolumeLevel;
}*/

//unsigned char GetSystemVolume()
//{
//    return VolumeArray[mixer.VolumeLevel];
//}


void HighVoltageMixerHandler()
{
    if(SystemError.StartBatteryHighError == START_BATTERY_HIGH_ERROR)
    {
        OnChipAudioPlay(200, 1, 0);
    }
}

void StopAuidoPlay()
{
    engine.status = ENGINE_STOP;
    TIM_Cmd(MIXER_CHANNEL0_TIMER, DISABLE);
    TIM_Cmd(MIXER_CHANNEL1_TIMER, DISABLE);
    TIM_Cmd(MIXER_OUT_TIMER, DISABLE);
    DAC_SetChannel2Data(DAC_Align_12b_L,  0x8000); 
    DAC->SWTRIGR = 0x02;   
    

    AmplifierControlHandler();
}

void StartAudioPlay()
{
    TIM_Cmd(MIXER_CHANNEL0_TIMER, ENABLE);
    TIM_Cmd(MIXER_CHANNEL1_TIMER, ENABLE);
    TIM_Cmd(MIXER_OUT_TIMER, ENABLE);
}



void MixerBleTopLevelHandler()
{
    FunctionalState DownloadFileFlag;
    DownloadFileFlag = IsAudioDownloading();

    if(mixer.MixerEnableFlag == DISABLE)
    {
        //Mixer关闭状态，之前在下载状态
        if(DownloadFileFlag == DISABLE || GetSystemTime() > GetLastDownloadMessageTime() + DOWNLOAD_TIME_OUT_TIME)
        {
            //下载完成或者超时中断
            StopDonwload();
            //TryChangeSoundIndex(mixer.SoundIndex);
            TryChangeVehileIndexLoop(mixer.SoundIndex, (bool)FALSE);
            StartAudioPlay();
            mixer.MixerEnableFlag = ENABLE;
        }
    }
    else
    {
        //Mixer开启状态
        if(DownloadFileFlag != DISABLE)
        {
            //开始进行音频下载
            StopAuidoPlay();
            mixer.MixerEnableFlag = DISABLE;
        }
    }
}


void VolumeHandler()
{
    static uint32_t VolumeHandleTime = 0;
    if(GetSystemTime() < VolumeHandleTime + 1000)
        return;
    VolumeHandleTime = GetSystemTime();

    if(RealVolume < mixer.VolumeLevel * 8)
        RealVolume++;
    else if(RealVolume > mixer.VolumeLevel * 8)
        RealVolume--;
}

void CalRealVolume()
{
    RealVolume = mixer.VolumeLevel * 8;
}

