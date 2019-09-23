#include "common.h"
#include "engine.h"
#include "MotorSpeed.h"
#include "ActionTick.h"
#include "AgentTest.h"
#include "mixer.h"
#include "SystemError.h"
#include "tea.h"
#include "engine.h"
#include "string.h"

#include "nrf_drv_timer.h"


#define DEFAULT_SLIENT_TIME_VALUE		5000  //默认5秒
#define MIN_SLIENT_TIME_VALUE			500//500ms
#define MAX_SLIENT_TIME_VALUE			60000//60s
#define NO_FLAME_OUT_VALUE				0xFFFFFFFE

//速度更新及混音计算周期最大最小值
#define SPEED_UPDATA_INTERVAL_MAX_TIME	1000
//速度更新及混音计算周期最大最小值
#define SPEED_UPDATA_INTERVAL_MIN_TIME	4
//速度更新及混音计算周期
#define SPEED_UPDATA_INTERVAL_UP			50
#define SPEED_UPDATA_INTERVAL_DOWN			60

//发动机播放过载声音的默认延时时间
#define ENGINE_OVERLOAD_SOUND_DEFAULT_DELAY_TIME		5000


//发动机力矩的最大值与最小值
#define MAX_ENGINE_MOMENT_VALUE						3000
#define MIN_ENGINE_MOMENT_VALUE						1
#define DEFAULT_ENGINE_MOMENT_VALUE					500





#define ENGINE_STOP_LAST_TIME		10000 //单位100ns
#define ENGINE_STOP_MIN_SPEED		300


//#define MAX_AUDIO_FILE_NUM_ONE_DIRECT		20
//#define MAX_AUDIO_FILE_NUM					(MAX_AUDIO_FILE_NUM_ONE_DIRECT * 2)


//const nrf_drv_timer_t MIXER_CHANNEL0_TIMER = NRF_DRV_TIMER_INSTANCE(2);  
//#define MixerChannel0IntHandler nrfx_timer_2_irq_handler
//#define TIMER_MIXER_CHANNEL0          NRF_TIMER2

//const nrf_drv_timer_t MIXER_CHANNEL1_TIMER = NRF_DRV_TIMER_INSTANCE(3);
//#define MixerChannel1IntHandler nrfx_timer_3_irq_handler
//#define TIMER_MIXER_CHANNEL1          NRF_TIMER3






struct EngineInfo engine = 
{
    .SampleRate = DEFAULT_SAMPLE_RATE
};
struct MixerChannelInfo MixerChannel[ENGINE_MIXER_CHANNEL_NUM] = 
{
    {.TimerInterval = (1000000000 / DEFAULT_SAMPLE_RATE)},
    {.TimerInterval = (1000000000 / DEFAULT_SAMPLE_RATE)},
};
struct MixerChannelInfo EngineStartStopVoiceChannel = 
{
    .TimerInterval = (1000000000 / DEFAULT_SAMPLE_RATE)
};

//void MixerChannel0IntHandlerNull(nrf_timer_event_t event_type, void            * p_context) {}
//void MixerChannel1IntHandlerNull(nrf_timer_event_t event_type, void            * p_context) {}


///********************初始化模块*************************/
//void EngineMixerChannelInit()
//{
//    uint32_t err_code = NRF_SUCCESS;
//    nrf_drv_timer_config_t timer_cfg;// = NRF_DRV_TIMER_DEFAULT_CONFIG;
//    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
//    timer_cfg.frequency = NRF_TIMER_FREQ_16MHz;
//    timer_cfg.interrupt_priority = IRQ_PROIRITY_TIMER_MIXER0;
//    timer_cfg.mode = NRF_TIMER_MODE_TIMER;
//    timer_cfg.p_context = NULL;
//    err_code = nrf_drv_timer_init(&MIXER_CHANNEL0_TIMER, &timer_cfg, MixerChannel0IntHandlerNull);
//    APP_ERROR_CHECK(err_code);

//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL0_TIMER, NRF_TIMER_CC_CHANNEL0, STANDARD_ARR_VALUE, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL0_TIMER, NRF_TIMER_CC_CHANNEL1, STANDARD_ARR_VALUE * 2, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK, true);
//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL0_TIMER, NRF_TIMER_CC_CHANNEL2, STANDARD_ARR_VALUE * 3, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE2_CLEAR_MASK, true);

//    nrf_drv_timer_enable(&MIXER_CHANNEL0_TIMER);    


//    err_code = nrf_drv_timer_init(&MIXER_CHANNEL1_TIMER, &timer_cfg, MixerChannel1IntHandlerNull);
//    APP_ERROR_CHECK(err_code);

//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL1_TIMER, NRF_TIMER_CC_CHANNEL0, STANDARD_ARR_VALUE, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL1_TIMER, NRF_TIMER_CC_CHANNEL1, STANDARD_ARR_VALUE * 2, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE1_CLEAR_MASK, true);
//    nrf_drv_timer_extended_compare(
//         &MIXER_CHANNEL1_TIMER, NRF_TIMER_CC_CHANNEL2, STANDARD_ARR_VALUE * 3, (nrf_timer_short_mask_t)NRF_TIMER_SHORT_COMPARE2_CLEAR_MASK, true);

//    nrf_drv_timer_enable(&MIXER_CHANNEL1_TIMER);    

//    
//}



/**************************引擎参数设定模块*********************************/
/*
void CalAccelerateArray(int16_t *AccelerationArray, uint16_t factor)
{
    //uint16_t DistFactor;
    int i;
    //若是标准的，则加速度矩阵不变
    //if(factor == 10000)
    //    return;

    
    for(i = 0; i < MAX_AUDIO_FILE_NUM_ONE_DIRECT; i++)
    {
        engine.AccelerateArray[i] = (int32_t)AccelerationArray[i] * factor / 10000;
    }

    
}
*/

//计算减速响应
void CalDecelerartion()
{
    engine.ThrottleDownResponse = ((uint32_t)engine.ThrottleDownResponseOri * 10000) / engine.EngineDecResponse / 2;
}



/*****************************进行引擎音频的解析*********************************/
void EngineMixerChannelVarInit(unsigned char SoundIndex)
{
    int i;


    for(i = 0; i < ENGINE_MIXER_CHANNEL_NUM; i++)
    {
        MixerChannel[i].Fp = OpenFile(ENGINE_INDEX(SoundIndex));
        MixerChannel[i].id = i;
        MixerChannel[i].AmplitudeFactor = 0;
        MixerChannel[i].Buff0ReadyFlag = 0;
        MixerChannel[i].Buff1ReadyFlag = 0;
        MixerChannel[i].Buff2ReadyFlag = 0;
        MixerChannel[i].DataPtr = 0;
        MixerChannel[i].ChannelStatus = CHANNEL_CLOSE;
        MixerChannel[i].NowBuffFlag = 0;
        MixerChannel[i].OriginSpeed = 0;
        MixerChannel[i].ReadFileCnt = 0;
        MixerChannel[i].SoundData = 0;
    }   
}


//头部长度数据量一定，和时间无关
//22050采样率有50ms
//44100采样率有25ms
int32_t CutAudioFileStartStop(uint32_t *StartAddr, uint32_t * DataCnt)
{
    //50:ms，MP3文件头部固定生成50ms数据，
    //22:为采样率22050下1ms的采样点，
    // 2:一个采样点占2Bytes
    *StartAddr = *StartAddr + 50 * (22050 / 1000) * 2;
    //去除头去除尾，均去除50ms的音频数据
    *DataCnt = *DataCnt - 50 * (22050 / 1000) * 2 * 2;
    return 0;
}

struct PointInfo
{
    int16_t Value;
    uint32_t position;
};


struct PointInfo GetTheMaxPoint(int16_t *buff, uint32_t cnt)
{
    struct PointInfo Point = {0, 0};
    uint32_t i;
    
    for(i = 0; i < cnt; i++)
    {
        if(buff[i] > Point.Value)
        {
            Point.Value = buff[i];
            Point.position = i;
        }
    }
    return Point;
}


#define MIN_INT16			-32768
#define THROUGH_THESHOLD		(MIN_INT16 / 20)//波谷的阈值，总体量程的5%
//从末尾开始寻找，找到第一个匹配的波谷
struct PointInfo GetTheLastTroughPoint(int16_t *buff, uint32_t cnt)
{
    bool IsTouchTheTheshold = false;
    struct PointInfo Point = {0, 0};
    Point.position = cnt - 1;

    while(Point.position > 0)
    {
        if(IsTouchTheTheshold == false)
        {
            //尝试找到符合波谷幅值的特征的点
            if(buff[Point.position] < THROUGH_THESHOLD)
            {
                IsTouchTheTheshold = true;
            }
        }
        else
        {
            //如果前面一个数据被后面一个数据大，表示找到了波谷
            //后面一个数据即为波谷
            if(buff[Point.position] > buff[Point.position + 1])
            {
                //找到波谷，后面位置为波谷，返回
                Point.position++;
                break;
            }
        }
        Point.position--;
    }
    Point.Value = buff[Point.position];
    return Point;
}

bool IsCrossZero(int16_t *buff, uint32_t DataCnt)
{
    int32_t i;
    bool Negative = false, Positive = false;
    for(i = 0; i < DataCnt; i++)
    {
        if(buff[i] < 0)
        {
            Negative = true;
        }
        else
        {
            Positive = true;
        }

        if(Negative != false && Positive != false)
        {
            return true;
        }
    }

    return false;
}

//找到正向过零点，返回过零后的数值为正的第一个点。
int32_t GetPositiveCrossZero(int16_t *buff, uint32_t DataCnt)
{
    int32_t i;
    for(i = 0; i < DataCnt - 1; i++)
    {
        if(buff[i] < 0 && buff[i + 1] >= 0)
        {
            return i + 1;
        }
    }

    return -1;
}


//找到反向过零点，返回过零前数值为正的第一个点
int32_t GetNegativeCrossZero(int16_t *buff, uint32_t DataCnt)
{
    int32_t i;
    for(i = 0; i < DataCnt - 1; i++)
    {
        if(buff[i] > 0 && buff[i + 1] <= 0)
        {
            return i;
        }
    }
    return -1;
}

//检查所有数值是否均大于0
bool CheckAllDataPositive(int16_t *buff, uint32_t DataCnt)
{
    int32_t i;
    for(i = 0; i < DataCnt; i++)
    {
        if(buff[i] < 0)
            return false;
    }
    return true;
}


//获取最小的正值
int32_t GetMinPositive(int16_t *buff, uint32_t DataCnt)
{
    int16_t MiniPositive = 32767;
    int32_t MinPosition = -1;
    int32_t i;
    for(i = 0; i < DataCnt; i++)
    {
        if(buff[i] >= 0 && buff[i] < MiniPositive)
        {
            MiniPositive = buff[i];
            MinPosition = i;
        }
    }
    return MinPosition;
}

int32_t GetMaxNegative(int16_t *buff, uint32_t DataCnt)
{
    int16_t MaxNegative = -30000;
    int32_t MaxPosition = -1;
    int32_t i;
    for(i = 0; i < DataCnt; i++)
    {
        if(buff[i] <= 0 && buff[i] > MaxNegative)
        {
            MaxNegative = buff[i];
            MaxPosition = i;
        }
    }
    return MaxPosition;
}

//找到MP3文件有效音频的尾部
//音频制作时，在snd转换成MP3前，会插入1帧1152字节最大数据，
//此处识别到最大数据段，返回最大数据段前的一个点
int32_t FindTailPoint(int16_t *buff, uint32_t DataCnt)
{
    int32_t ptr;
    bool FindSignature = false;
    int32_t ContinueSignature = 0;
    ptr = DataCnt - 1;

    //尝试找到标识数据段
    while(ptr > 0)
    {
        if(buff[ptr] > 25000)
        {
            ContinueSignature++;
            if(ContinueSignature >= 20)
            {
                FindSignature = true;
                break;
            }
        }
        else
        {
            ContinueSignature = 0;
        }
        ptr--;
    }

    if(FindSignature == false)
    {
        //没有找到特征区域，
        return DataCnt - 1;
    }

    //找到了特征数据段
    while(ptr > 0)
    {
        //尝试从后往前找到第一个正向过零点，音频截取的时候需要截在波峰
        if(buff[ptr] >= 0 && buff[ptr - 1] < 0)
        {
            break;
        }
        ptr--;
    }

    return ptr;
}



#define CUT_BUFF_SIZE				(2304)//用于音频截取的缓冲大小，缓冲类型为int16_t类型，MP3一帧1152字节数据
int16_t CutTemp[2304 - 2000];



#define OFFSET_HEAD_START_CUT               (1100)// 22050波特率下，约50ms的数据，在生成MP3文件时，会引入，需要去除    
#define DATA_CNT_HAED_CUT_BUFF              25      
int32_t CutAudioOneSpeed(struct FileInfo *Fp, uint32_t *StartAddr, uint32_t * DataCnt)
{
    int16_t buff[CUT_BUFF_SIZE];
    int32_t position = -1;
    int32_t EndPosition;

    //struct PointInfo MaxPoint;
    //struct PointInfo StartPoint;
    //struct PointInfo LastThroghPoint;
    //struct PointInfo EndPoint;
    
    /*************头部处理**************/
    ReadFile(Fp, *StartAddr + OFFSET_HEAD_START_CUT * sizeof(int16_t), (uint8_t *)buff, DATA_CNT_HAED_CUT_BUFF * sizeof(int16_t));
    DeconfuseWav_16bit((uint8_t *)buff, DATA_CNT_HAED_CUT_BUFF * sizeof(int16_t));

    if(IsCrossZero(buff, DATA_CNT_HAED_CUT_BUFF) == true)
    {
        //存在过零
        position = GetPositiveCrossZero(buff, DATA_CNT_HAED_CUT_BUFF);//尝试寻找正向过零
        if(position == -1)
        {
            //没有正向过零，尝试寻找反向过零
            position = GetNegativeCrossZero(buff, DATA_CNT_HAED_CUT_BUFF);//尝试寻找反向过零
            if(position == -1)
            {
                //未找到正向过零和反向过零，理论上不应该出现
            }
        }
    }
    else
    {
        //不存在过零
        if(CheckAllDataPositive(buff, DATA_CNT_HAED_CUT_BUFF) == true)
        {
            //所有数据均为正
            position = GetMinPositive(buff, DATA_CNT_HAED_CUT_BUFF);
        }
        else
        {
            //所有数据均为负
            position = GetMaxNegative(buff, DATA_CNT_HAED_CUT_BUFF);
        }
    }

    if(position == -1)
        position = 0;

    /*********尾部处理***********/
    ReadFile(Fp, *StartAddr +  *DataCnt - CUT_BUFF_SIZE * sizeof(int16_t), (uint8_t *)buff, CUT_BUFF_SIZE * sizeof(int16_t));
    DeconfuseWav_16bit((uint8_t *)buff, CUT_BUFF_SIZE * sizeof(int16_t));

    memcpy(CutTemp, &buff[2000], (2304 - 2000) * 2);


    EndPosition = FindTailPoint(buff, CUT_BUFF_SIZE);

/*
    //找到最后一个波谷的最低的一个点
    LastThroghPoint = GetTheLastTroughPoint(buff, CUT_BUFF_SIZE);
    //找到最后一个波谷后面的过零点
    EndPoint = LastThroghPoint;
    while(EndPoint.position < CUT_BUFF_SIZE)
    {
        if(buff[EndPoint.position] >= 0)
            break;

        EndPoint.position++;
    }
    //LastValue = buff[EndPoint.position];
    //往回退一个点，上一个点才是过零前的一个点
    EndPoint.position--;
    EndPoint.Value = buff[EndPoint.position];
*/

    //计算起始地址
    *StartAddr = *StartAddr + (OFFSET_HEAD_START_CUT + position) * sizeof(int16_t);
    //计算减去的数据总量
    //*DataCnt = *DataCnt -((StartPoint.position + CUT_BUFF_SIZE - EndPoint.position + 1) * sizeof(int16_t));
    *DataCnt = *DataCnt -((OFFSET_HEAD_START_CUT + position + CUT_BUFF_SIZE - EndPosition + 1) * sizeof(int16_t));


    return 0;
    
}






int GetEngineInfo(unsigned char SoundIndex)
{
    unsigned char buff[512];
    int i, j;
    uint16_t StartSector;

    
    if(File[ENGINE_INDEX(SoundIndex)].EnableFlag == DISABLE)
    {
        return 4;
    }

    ReadFile(&File[ENGINE_INDEX(SoundIndex)], 0, buff, 512);
    engine.ForwardAudioFileNum = buff[FORWARD_AUDIO_CNT_OFFSET];
    engine.BackwardAudioFileNum = buff[BACKWARD_AUDIO_CNT_OFFSET];
    if(engine.ForwardAudioFileNum == 0)
        return 3;
    else if(engine.ForwardAudioFileNum >= MAX_AUDIO_FILE_NUM_ONE_DIRECT || engine.BackwardAudioFileNum >= MAX_AUDIO_FILE_NUM_ONE_DIRECT)
        return 2;

    //获取前进的语音信息
    for(i = 0; i < engine.ForwardAudioFileNum; i++)
    {
        engine.ForwardSpeedArray[i] = buff[SPEED_TABLE_OFFSET + i * 2] * 256;
        engine.ForwardSpeedArray[i] += buff[SPEED_TABLE_OFFSET + i * 2 + 1];
        
        engine.ForwardDataCntArray[i] = buff[AUDIO_LENGTH_OFFSET + i * 4] << 24;
        engine.ForwardDataCntArray[i] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 1] << 16;
        engine.ForwardDataCntArray[i] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 2] << 8;
        engine.ForwardDataCntArray[i] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 3];

        StartSector = buff[AUDIO_START_SECTOR_OFFSET + i * 2] << 8;
        StartSector += buff[AUDIO_START_SECTOR_OFFSET + i * 2 + 1];
        engine.ForwardAudioStartAddrArray[i] = (uint32_t)StartSector * 512;

        if(engine.ForwardSpeedArray[i] == 0 || engine.ForwardDataCntArray[i] == 0 || engine.ForwardDataCntArray[i] >= FS_ONE_FILE_MAX_SIZE ||
            engine.ForwardAudioStartAddrArray[i] == 0 || engine.ForwardAudioStartAddrArray[i] >= FS_SUPPORT_MAX_SIZE)
        {
            return 1;
        }
    }

    //获取倒车的语音信息
    for(j = 0; i < engine.ForwardAudioFileNum + engine.BackwardAudioFileNum; i++)
    {
        engine.BackwardSpeedArray[j] = buff[SPEED_TABLE_OFFSET + i * 2] * 256;
        engine.BackwardSpeedArray[j] += buff[SPEED_TABLE_OFFSET + i * 2 + 1];
        engine.BackwardSpeedArray[j] = 0 - engine.BackwardSpeedArray[j];
        
//        engine.BackwardDataCntArray[j] = buff[AUDIO_LENGTH_OFFSET + i * 4] << 24;
//        engine.BackwardDataCntArray[j] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 1] << 16;
//        engine.BackwardDataCntArray[j] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 2] << 8;
//        engine.BackwardDataCntArray[j] += buff[AUDIO_LENGTH_OFFSET + i * 4 + 3];


//        StartSector = buff[AUDIO_START_SECTOR_OFFSET + i * 2] << 8;
//        StartSector += buff[AUDIO_START_SECTOR_OFFSET + i * 2 + 1];
//        engine.BackwardAudioStartAddrArray[i] = (uint32_t)StartSector * 512;

        if(engine.BackwardSpeedArray[j] == 0)
        {
            return 1;
        }
        j++;
    }

    //若有过载语音，并且过载功能被关闭，则需要将正传语音数量减1从而当速度高于最高速时能够正常工作
    if(engine.OverloadExistFlag == ENABLE && engine.OverloadEnableFlag == DISABLE)
    {
        engine.ForwardAudioFileNum--;
    }
    engine.ForwardLowSpeed = engine.ForwardSpeedArray[0];
    engine.ForwardHighSpeed = engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 1];

    StartSector = buff[START_SOUND_START_SECTOR_OFFSET] << 8;
    StartSector += buff[START_SOUND_START_SECTOR_OFFSET + 1];
    engine.StartAudioStartAddr = StartSector * 512;
    engine.StartAudioDataCnt = buff[START_SOUND_LENGTH_OFFSET] << 24;
    engine.StartAudioDataCnt += buff[START_SOUND_LENGTH_OFFSET + 1] << 16;
    engine.StartAudioDataCnt += buff[START_SOUND_LENGTH_OFFSET + 2] << 8;
    engine.StartAudioDataCnt += buff[START_SOUND_LENGTH_OFFSET + 3];
    if(engine.StartAudioDataCnt == 0 || engine.StartAudioDataCnt >= FS_ONE_FILE_MAX_SIZE ||
            engine.StartAudioStartAddr == 0 || engine.StartAudioStartAddr >= FS_SUPPORT_MAX_SIZE)
    {
        return 5;
    }

    StartSector = buff[STOP_SOUND_START_SECTOR_OFFSET] << 8;
    StartSector += buff[STOP_SOUND_START_SECTOR_OFFSET + 1];
    engine.StopAudioStartAddr = StartSector * 512;
    engine.StopAudioDataCnt = buff[STOP_SOUND_LENGTH_OFFSET] << 24;
    engine.StopAudioDataCnt += buff[STOP_SOUND_LENGTH_OFFSET + 1] << 16;
    engine.StopAudioDataCnt += buff[STOP_SOUND_LENGTH_OFFSET + 2] << 8;
    engine.StopAudioDataCnt += buff[STOP_SOUND_LENGTH_OFFSET + 3];

    //得到引擎文件的采样率
    memcpy(&engine.SampleRate, buff + AUDIO_SAMPLE_RATE_OFFSET, AUDIO_SAMPLE_RATE_LENGTH);
    if(engine.SampleRate != 44100 && engine.SampleRate != 22050)
    {
        engine.SampleRate = 44100;
    }
    //memcpy(&engine.BitRate, buff + AUDIO_BIT_RATE_OFFSET, AUDIO_BIT_RATE_LENGTH);
    

    engine.ChannelNum = 0;
    engine.SpeedStage = 0;
    engine.LowSpeedChannel = 0;
    engine.HighSpeedChannel = 1;

    engine.BackwardLowSpeed = engine.BackwardSpeedArray[0];
    engine.BackwardHighSpeed = engine.BackwardSpeedArray[engine.BackwardAudioFileNum - 1];
    engine.ForwardLowSpeed = engine.ForwardSpeedArray[0];
    engine.ForwardHighSpeed = engine.ForwardSpeedArray[engine.ForwardAudioFileNum - 1];

    
    MotorSpeed.SlientSpeed = engine.ForwardLowSpeed / 2;


    for(i = 0; i < engine.ForwardAudioFileNum; i++)
    {
        CutAudioOneSpeed(&File[ENGINE_INDEX(SoundIndex)], &engine.ForwardAudioStartAddrArray[i], &engine.ForwardDataCntArray[i]);
    }
    
    CutAudioFileStartStop(&engine.StartAudioStartAddr, &engine.StartAudioDataCnt);
    if(engine.StopAudioDataCnt != 0 && engine.StopAudioStartAddr != 0)
    {
        CutAudioFileStartStop(&engine.StopAudioStartAddr, &engine.StopAudioDataCnt);
    }
    return 0;
}

void EngineStopWorking()
{
    engine.StartAudioStartAddr = 0;
    engine.StartAudioDataCnt = 0;
}


int AnalyzeEngineFile(unsigned char SoundIndex)
{
    int ret;
    engine.status = ENGINE_STOP;
    MotorSpeed.RealSpeed = 0;
    MotorSpeed.DistSpeed = 0;
    EngineMixerChannelVarInit(SoundIndex);
    ret = GetEngineInfo(SoundIndex);
    if(ret != 0)
    {
        SetSystmError(AUDIO_FILE_ERROR);
        EngineStopWorking();
    }
    else
    {
        ClearSystemError(AUDIO_FILE_ERROR);
    }
    GearBoxInit();
    return ret;
}

#define MAX_VOLUME_SLOPE            20
#define PERCENT_STEP_BELOW_MIDDLE   500
#define PERCENT_STEP_ABOVE_MIDDLE   500
uint16_t CalVolumeSlopePercent(int16_t RealSpeed)
{
    uint32_t VolumeSlopePercent;

    if(engine.status == ENGINE_RUNNING || engine.status == ENGINE_STARTING)
    {
        if(mixer.VolumeSlope > MAX_VOLUME_SLOPE)
        {
            mixer.VolumeSlope = 0;
        }

    
        //斜率分为10档，每一档会有对应的最高速的音量比例和最低速的音量比例
        //最低速的比例关系：0）1；1）0.95；2）0.9。。。。。。。。9）0.55；10）0.5
        //最高速的比例关系：0）1；2）1.1；2）1.2.............9)1.9;10)2.0
        if(RealSpeed < 0)
        {
            RealSpeed = -RealSpeed;
        }

        if(RealSpeed < engine.ForwardLowSpeed)
        {
            return 10000 - PERCENT_STEP_BELOW_MIDDLE * mixer.VolumeSlope;
        }
        else if(RealSpeed > engine.ForwardHighSpeed)
        {
            return 10000 + PERCENT_STEP_ABOVE_MIDDLE * mixer.VolumeSlope;
        }
        else
        {
            int16_t MiddleSpeed;
            MiddleSpeed = (engine.ForwardHighSpeed + engine.ForwardLowSpeed) >> 1;
            if(RealSpeed <= MiddleSpeed)
            {
                uint32_t LowSpeedPercent = 10000 - PERCENT_STEP_BELOW_MIDDLE * mixer.VolumeSlope;
                uint32_t SpeedPercent = (RealSpeed - engine.ForwardLowSpeed) * 10000 / (MiddleSpeed - engine.ForwardLowSpeed);
                VolumeSlopePercent = LowSpeedPercent + SpeedPercent * (10000 - LowSpeedPercent) / 10000;
            }
            else
            {
                uint32_t HighSpeedPercent = 10000 + PERCENT_STEP_ABOVE_MIDDLE * mixer.VolumeSlope;
                uint32_t SpeedPercent = (RealSpeed - MiddleSpeed) * 10000 / (engine.ForwardHighSpeed - MiddleSpeed);
                VolumeSlopePercent = 10000 + SpeedPercent * (HighSpeedPercent - 10000) / 10000;
            }
        }

        
    }
    else
    {
        VolumeSlopePercent = 10000;
    }
    return VolumeSlopePercent;
}


/*********************引擎速度计算模块*************************/
//其中包含了对于RC信号计算，刹车状态判断，换挡的处理
//引擎响应处理，最终得出一个实际速度
void CalSpeedAndBrakeAndGearBoxHandler()
{
    static uint32_t MixTimeInterval = 0;

    //加减速的Δ不同，可以根据配置形成不同的加减速曲线
    if(((MotorSpeed.VehicleStatu == GO_FORWARD) && (MotorSpeed.DistSpeed >= MotorSpeed.RealSpeed) && (GetSystemTime() - MixTimeInterval >= (engine.ThrottleUpResponse >> REAL_SPEED_CAL_MULT_SHIFT)))
        || ((MotorSpeed.VehicleStatu == GO_FORWARD) && (MotorSpeed.DistSpeed < MotorSpeed.RealSpeed) && (GetSystemTime() - MixTimeInterval >= engine.ThrottleDownResponse))
        || ((MotorSpeed.VehicleStatu == GO_BACKWARD) && (MotorSpeed.DistSpeed <= MotorSpeed.RealSpeed) && (GetSystemTime() - MixTimeInterval >= (engine.ThrottleUpResponse >> REAL_SPEED_CAL_MULT_SHIFT)))
        || ((MotorSpeed.VehicleStatu == GO_BACKWARD) && (MotorSpeed.DistSpeed > MotorSpeed.RealSpeed) && (GetSystemTime() - MixTimeInterval >= engine.ThrottleDownResponse))
        || ((MotorSpeed.VehicleStatu == BREAK) && (GetSystemTime() - MixTimeInterval >= engine.ThrottleDownResponse)))
    {
        MixTimeInterval = GetSystemTime();
        
        
        MotorSpeed.DistSpeed = CalDistSpeed();


        
        if(engine.status == ENGINE_RUNNING)
        {
            /*if(MotorSpeed.GearBox.GearBoxEnableFlag == DISABLE)
            {
                 //根据目标速度计算出当前实际速度
                MotorSpeed.RealSpeed = GetMotorRealSpeedWithoutGearbox(MotorSpeed.DistSpeed, MotorSpeed.RealSpeed);
            }
            else
            {
                MotorSpeed.RealSpeed = GetMotorRealSpeedWithGearBox(MotorSpeed.DistSpeed, MotorSpeed.RealSpeed);
            }*/
           
            ////在引擎正常运作的时候进行混音
            //SetMixerChannel(Speed);
            MotorSpeed.RealSpeed = GetMotorRealSpeedWithoutGearbox(MotorSpeed.DistSpeed, MotorSpeed.RealSpeed);
        }

        mixer.VolumeSlopePercent = CalVolumeSlopePercent(MotorSpeed.RealSpeed);
    }
}



/********************************根据实际速度计算mixer比例**************************************/
#define STAGE_MIX_PERCENT                       6 //单位：10%，在一个速度区间内，进行两段音频混音的区间百分比
void SetMixerStatus(int16_t speed)
{
    int i;
    uint8_t NowSpeedStage = 0;
    if(MotorSpeed.VehicleStatu == GO_FORWARD || 
          (MotorSpeed.BreakDirect == FORWARD_BREAK && MotorSpeed.VehicleStatu == BREAK))
    //if(speed >= 0)
    {
        //获取当前速度所在的语音段。2段有重合，则取高的那段
        //减一为了不让语音越界，到最后一个已经无需判断，必定是最后一个了
        for(i = 0; i < engine.ForwardAudioFileNum - 1; i++)
        {
            if(speed > engine.ForwardSpeedArray[i])//速度等于的时候，算较低速的阶段
            {
                NowSpeedStage++;//此处验证，三段时，速度很高是否就到第三段。
            }
            else
            {
                break;
            }
        }
        engine.SpeedStage = NowSpeedStage;
    
        
        if(NowSpeedStage > 0)
        {  
            engine.UnMixSpeed = (engine.ForwardSpeedArray[NowSpeedStage] - engine.ForwardSpeedArray[NowSpeedStage - 1]) * STAGE_MIX_PERCENT / 10 +
                                                       engine.ForwardSpeedArray[NowSpeedStage - 1];
            //需要对速度是否落在混音区间进行判断
            if(speed >= engine.UnMixSpeed)
            {//当速度落在不需要语音叠加的区间
                engine.ChannelNum = 1;
            }
            else
            {//速度落在需要语音叠加的区间
                engine.ChannelNum = 2;
            }
        }
        else
        {
            //若速度落在第0阶段，则必定只有一个通道
            engine.ChannelNum = 1;
        }
    }
    else
    {
        speed = 0 - speed;
        //获取当前速度所在的语音段。2段有重合，则取高的那段
        //减一为了不让语音越界，到最后一个已经无需判断，必定是最后一个了
        for(i = 0; i < engine.BackwardAudioFileNum - 1; i++)
        {
            if(speed > engine.BackwardSpeedArray[i])//速度等于的时候，算较低速的阶段
            {
                NowSpeedStage++;//此处验证，三段时，速度很高是否就到第三段。
            }
            else
            {
                break;
            }
        }
        engine.SpeedStage = NowSpeedStage;
    
        
        if(NowSpeedStage > 0)
        {  
            engine.UnMixSpeed = (engine.BackwardSpeedArray[NowSpeedStage] - engine.BackwardSpeedArray[NowSpeedStage - 1]) * STAGE_MIX_PERCENT / 10 +
                                                       engine.BackwardSpeedArray[NowSpeedStage - 1];
            //需要对速度是否落在混音区间进行判断
            if(speed >= engine.UnMixSpeed)
            {//当速度落在不需要语音叠加的区间
                engine.ChannelNum = 1;
            }
            else
            {//速度落在需要语音叠加的区间
                engine.ChannelNum = 2;
            }
        }
        else
        {
            //若速度落在第0阶段，则必定只有一个通道
            engine.ChannelNum = 1;
        }
    }
}


void OpenAudioFile(struct MixerChannelInfo *Channel, uint16_t Stage)
{
    //uint32_t ModNum;
    //uint32_t MinusCnt;
    Channel->ChannelStatus = CHANNEL_OPEN;
    Channel->TotalDataCnt = engine.ForwardDataCntArray[Stage];
    Channel->OriginSpeed = engine.ForwardSpeedArray[Stage];
    Channel->AudioStartAddress = engine.ForwardAudioStartAddrArray[Stage];

    Channel->Buff0ReadyFlag = 0;
    Channel->Buff1ReadyFlag = 0;
    Channel->Buff2ReadyFlag = 0;
    Channel->NowBuffFlag = 0;
    Channel->ReadFileCnt = 0;

    
    if(Stage == engine.ForwardAudioFileNum - 1 && engine.OverloadExistFlag == ENABLE && engine.OverloadEnableFlag == ENABLE)
    {
        //如果是过载的声音，则从头部开始播放
        Channel->ReadFileCnt = 0;
    }
    else
    {
        //其他转速的声音，从随机位置开始播放
//        ModNum = GetSystemTime() % Channel->TotalDataCnt;
//        MinusCnt = ModNum % AUDIO_DATA_READ_CNT;
//        Channel->ReadFileCnt = ModNum - MinusCnt;

        Channel->ReadFileCnt = 0;
    }



}



void TryOpenAudioFile(struct MixerChannelInfo *Channel, uint16_t Stage)
{
    if(Channel->ChannelStatus == CHANNEL_CLOSE)
    {
        OpenAudioFile(Channel, Stage);
        
    }
}

void TryCloseAudioFile(struct MixerChannelInfo *Channel)
{
    if(Channel->ChannelStatus == CHANNEL_OPEN)
    {
        Channel->Buff0ReadyFlag = 0;
        Channel->Buff1ReadyFlag = 0;
        Channel->Buff2ReadyFlag = 0;
        Channel->DataPtr = 0;
        Channel->ReadFileCnt = 0;
        Channel->SoundData = 0;
        Channel->ChannelStatus = CHANNEL_CLOSE;

    }
}

void MixAudio(int16_t MotorSpeed)
{
    uint32_t factor_temp;

    if(MotorSpeed < 0)
    {
        MotorSpeed = 0 - MotorSpeed;
    }

    //若声音比怠速声音小，则播放怠速声音
    if(MotorSpeed < engine.ForwardLowSpeed)
    {
        MotorSpeed = engine.ForwardLowSpeed;
    }

    
    //无混音
    if(engine.ChannelNum == 1)
    {
         if(MixerChannel[engine.LowSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage])
         {
              //通道零状态符合
              MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE;
              MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = 0;

              if(engine.LowSpeedChannel == 0)
              {
                  MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
              }   
              else
                  MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;

              TryCloseAudioFile(&MixerChannel[engine.HighSpeedChannel]);

         }
         else if(MixerChannel[engine.HighSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage])
         {
              //通道一状态符合
              MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = 0;
              MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE;

              if(engine.HighSpeedChannel == 1)
                  MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;
              else
                  MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;

              TryCloseAudioFile(&MixerChannel[engine.LowSpeedChannel]);
         }
         else
         {
             TryCloseAudioFile(&MixerChannel[engine.LowSpeedChannel]);
             TryCloseAudioFile(&MixerChannel[engine.HighSpeedChannel]);

             engine.LowSpeedChannel = 0;
             engine.HighSpeedChannel = 1;

             OpenAudioFile(&MixerChannel[engine.LowSpeedChannel], engine.SpeedStage);
             
             //两个通道均不符合，则选择通道0
             MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE;
             MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = 0;

             
             MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
         }
    }
    else if(engine.ChannelNum == 2)//需要混音
    {
        //两个通道状态均符合
        if(MixerChannel[engine.LowSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage - 1] &&
            MixerChannel[engine.HighSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage])
        {
            //对文件尝试打开，若已经打开则无需打开
            //如下情况会打开文件
            //进入到最低速度区间，然后提速
            //进入到最高速度区间，然后降速
            TryOpenAudioFile(&MixerChannel[engine.LowSpeedChannel], engine.SpeedStage - 1);
            TryOpenAudioFile(&MixerChannel[engine.HighSpeedChannel], engine.SpeedStage);

            
            factor_temp = (MotorSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed) * PERCENT_MIXER_BASE / 
                                  (engine.UnMixSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed);   
            MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - ((uint32_t)((uint32_t)factor_temp * (uint32_t)factor_temp) >> PERCENT_MIXER_BASE_SHIFT);
            MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - (((uint32_t)(PERCENT_MIXER_BASE - factor_temp) * (uint32_t)(PERCENT_MIXER_BASE - factor_temp)) >> PERCENT_MIXER_BASE_SHIFT);  
            //MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - factor_temp;
            //MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = factor_temp;
            
            if(engine.LowSpeedChannel == 0)
            {
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;
            }
            else
            {
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;
            }
        }
        //当速度变慢时，进入低一档时
        else if(MixerChannel[engine.LowSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage])
        {
            TryCloseAudioFile(&MixerChannel[engine.HighSpeedChannel]);
            OpenAudioFile(&MixerChannel[engine.HighSpeedChannel], engine.SpeedStage - 1);

            if(engine.LowSpeedChannel == 0)
            {
                engine.LowSpeedChannel = 1;
                engine.HighSpeedChannel = 0;
            }
            else
            {
                engine.LowSpeedChannel = 0;
                engine.HighSpeedChannel = 1;
            }
            
           

            factor_temp = (MotorSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed) * PERCENT_MIXER_BASE / 
                                  (engine.UnMixSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed);
            MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - ((uint32_t)((uint32_t)factor_temp * (uint32_t)factor_temp) >> PERCENT_MIXER_BASE_SHIFT);
            MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - (((uint32_t)(PERCENT_MIXER_BASE - factor_temp) * (uint32_t)(PERCENT_MIXER_BASE - factor_temp)) >> PERCENT_MIXER_BASE_SHIFT);  
            //MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - factor_temp;
            //MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = factor_temp;
            
            if(engine.LowSpeedChannel == 0)
            {
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;     
            }
            else
            {
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed;
            }
        }
        //当速度变快，进入高一档时
        else if(MixerChannel[engine.HighSpeedChannel].OriginSpeed == engine.ForwardSpeedArray[engine.SpeedStage - 1])
        {
            TryCloseAudioFile(&MixerChannel[engine.LowSpeedChannel]);
            OpenAudioFile(&MixerChannel[engine.LowSpeedChannel], engine.SpeedStage);
            if(engine.LowSpeedChannel == 0)
            {
                engine.LowSpeedChannel = 1;
                engine.HighSpeedChannel = 0;
            }
            else
            {
                engine.LowSpeedChannel = 0;
                engine.HighSpeedChannel = 1;
            }

            
            factor_temp = (MotorSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed) * PERCENT_MIXER_BASE / 
                                  (engine.UnMixSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed);
            MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - ((uint32_t)((uint32_t)factor_temp * (uint32_t)factor_temp) >> PERCENT_MIXER_BASE_SHIFT);
            MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - (((uint32_t)(PERCENT_MIXER_BASE - factor_temp) * (uint32_t)(PERCENT_MIXER_BASE - factor_temp)) >> PERCENT_MIXER_BASE_SHIFT);  
            //MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - factor_temp;
            //MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = factor_temp;
            
            if(engine.LowSpeedChannel == 0)
            {
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed; 
            }
            else
            {
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed; 
            }
        }
        //当速度图片时?应该不会发生
        //以下else不会发生
        else
        {
            TryCloseAudioFile(&MixerChannel[engine.LowSpeedChannel]);
            TryCloseAudioFile(&MixerChannel[engine.HighSpeedChannel]);
            engine.LowSpeedChannel = 0;
            engine.HighSpeedChannel = 1;
            OpenAudioFile(&MixerChannel[engine.LowSpeedChannel], engine.SpeedStage - 1);
            OpenAudioFile(&MixerChannel[engine.HighSpeedChannel], engine.SpeedStage);


            factor_temp = (MotorSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed) * PERCENT_MIXER_BASE / 
                                  (engine.UnMixSpeed - MixerChannel[engine.LowSpeedChannel].OriginSpeed);
            MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - ((uint32_t)((uint32_t)factor_temp * (uint32_t)factor_temp) >> PERCENT_MIXER_BASE_SHIFT);
            MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - (((uint32_t)(PERCENT_MIXER_BASE - factor_temp) * (uint32_t)(PERCENT_MIXER_BASE - factor_temp)) >> PERCENT_MIXER_BASE_SHIFT);  
            //MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - factor_temp;
            //MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = factor_temp;
            
            if(engine.LowSpeedChannel == 0)
            {
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed; 
            }
            else
            {
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / MotorSpeed;
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.HighSpeedChannel].OriginSpeed / MotorSpeed; 
            }
        }
    }
}


void SetMixerChannel(int16_t speed)
{
    //static int16_t LastSpeed = 0;
    //if(LastSpeed == speed)
    //{
    //    return;
    //}

    //LastSpeed = speed;
    SetMixerStatus(speed);
    MixAudio(speed);
}


/******************************引擎启动处理***********************************/
uint32_t EngineStartHandler()
{

    int i;

    if(engine.StartAudioStartAddr == 0 && engine.StartAudioDataCnt == 0)
    {
        return 0;
    }

    //试图关闭混音通道
    for(i = 0;  i < ENGINE_MIXER_CHANNEL_NUM; i++)
    {
        //对于初始速度进行清零，下次进入混音时，可重新开始打开语音文件，
        //不会造成语音文件重复打开的问题
        MixerChannel[i].OriginSpeed = 0;
        //试图关闭混音通道的音频文件句柄，原先应该已经进行了关闭
        TryCloseAudioFile(&MixerChannel[i]);
    }

    EngineStartStopVoiceChannel.Fp = OpenFile(ENGINE_INDEX(mixer.SoundIndex));

    //对混音通道的数据进行读取，在启动声音播放完成后能够立刻播放,读取完成后对通道比重清零，否则播放启动前会有杂音
    //引擎工作的声音
    //SetMixerChannel(GetMotorSpeed());
    //获取最低速的语音
    //使用最低速+1的速度，是为了能够第一时间获取最低两通道的数据，否则从怠速往上加时会有哒一声
    SetMixerChannel(engine.ForwardSpeedArray[0] + 1);
    if(MixerChannel[0].AmplitudeFactor != 0)
    {
        //混音通道的比重都设置为0，并保存
        MixerChannel[0].AmplitudeFactor = 0;
        AudioFileReadHandler(&MixerChannel[0]);
        AudioFileReadHandler(&MixerChannel[0]);
        AudioFileReadHandler(&MixerChannel[0]);
    }
    if(MixerChannel[1].AmplitudeFactor != 0)
    {
        //混音通道的比重都设置为0，并保存
        MixerChannel[1].AmplitudeFactor = 0;
        AudioFileReadHandler(&MixerChannel[1]);
        AudioFileReadHandler(&MixerChannel[1]);
        AudioFileReadHandler(&MixerChannel[1]);
    }
    //对启动通道的控制变量进行初始化

    EngineStartStopVoiceChannel.AudioStartAddress = engine.StartAudioStartAddr;
    EngineStartStopVoiceChannel.TotalDataCnt = engine.StartAudioDataCnt;
    

    EngineStartStopVoiceChannel.NowBuffFlag = 0;
    EngineStartStopVoiceChannel.ReadFileCnt = 0;
    EngineStartStopVoiceChannel.NowBuffFlag = 0;
    EngineStartStopVoiceChannel.Buff0ReadyFlag = 0;
    EngineStartStopVoiceChannel.Buff1ReadyFlag = 0;
    EngineStartStopVoiceChannel.Buff2ReadyFlag = 0;
    EngineStartStopVoiceChannel.DataPtr = 0;
    EngineStartStopVoiceChannel.ReadFileCnt = 0;
    EngineStartStopVoiceChannel.SoundData = 0;
    EngineStartStopVoiceChannel.AmplitudeFactor = PERCENT_MIXER_BASE;

    //设置采样频率为44100
    //将定时器的值暂存，该值是正常播放时，通道一的播放速率
    //用于启动声音播放完成后恢复用
    //Engine.Tim7ArrBackup = MIXER_CHANNEL1_TIMER->ARR;
    //混音通道的比重都设置为0，并保存
    //Engine.AmplitudeFactorCh0Backup = MixerChannel[0].AmplitudeFactor;
    //Engine.AmplitudeFactorCh1Backup = MixerChannel[1].AmplitudeFactor;
    //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = (uint32_t)STANDARD_ARR_VALUE;
    EngineStartStopVoiceChannel.TimerInterval = (1000000000 / engine.SampleRate);
    MixerChannel[0].SoundData = 0;
    MixerChannel[1].SoundData = 0;
    

    engine.status = ENGINE_STARTING;

    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    AudioFileReadHandler(&EngineStartStopVoiceChannel);

    //开始启动后，实际速度值为启动速度的阈值
    //MotorSpeed.RealSpeed = engine.ForwardSpeedArray[0] * 9 / 10;
    //MotorSpeed.RealSpeed = engine.ForwardSpeedArray[0];

    
    return 0;
}


void EngineStartingHandler()
{
    uint32_t distance;
    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    AudioFileReadHandler(&MixerChannel[0]);
    //播放一遍完成后，读取指正指向文件0偏移处，则表示播放完成
    if(EngineStartStopVoiceChannel.ReadFileCnt == 0)
    {
        //还原混音通道1的频率
        //MIXER_CHANNEL1_TIMER->ARR = Engine.Tim7ArrBackup;
        //还原原先混音通道的比重
        //MixerChannel[0].AmplitudeFactor = Engine.AmplitudeFactorCh0Backup;
        //MixerChannel[1].AmplitudeFactor = Engine.AmplitudeFactorCh1Backup;
        //设定两个通道的速度，防止在计算混音时速度不匹配导致引入杂音
        //MixerChannel[0].OriginSpeed = 0;
        //MixerChannel[1].OriginSpeed = MixerStatus.ForwardSpeedArray[0];
        engine.status = ENGINE_RUNNING;

        //对启动通道的控制变量进行初始化
        EngineStartStopVoiceChannel.NowBuffFlag = 0;
        EngineStartStopVoiceChannel.ReadFileCnt = 0;
        EngineStartStopVoiceChannel.NowBuffFlag = 0;
        EngineStartStopVoiceChannel.Buff0ReadyFlag = 0;
        EngineStartStopVoiceChannel.Buff1ReadyFlag = 0;
        EngineStartStopVoiceChannel.Buff2ReadyFlag = 0;
        EngineStartStopVoiceChannel.DataPtr = 0;
        EngineStartStopVoiceChannel.ReadFileCnt = 0;
        EngineStartStopVoiceChannel.SoundData = 0;

        //设置当前速度为怠速
        //发动机启动后，先由怠速运行，根据风门加速到相应速度
        //MotorSpeed.RealSpeed = 0;//SystemConfig.LowSpeed;
        MotorSpeed.RealSpeed = engine.ForwardLowSpeed;
        engine.InFlameOutTime = GetSystemTime();
        //MotorSpeed.RealSpeedChangeFlag = 1;

        //更新静音时间，防止启动后油门在怠速区间时，直接进入stop
        engine.InFlameOutTime = GetSystemTime();
    }
    else if(EngineStartStopVoiceChannel.ReadFileCnt >= ((EngineStartStopVoiceChannel.TotalDataCnt >> 3) * 6))
    {
        //启动声音的3/4处开始混入怠速的声音
        distance = EngineStartStopVoiceChannel.ReadFileCnt - ((EngineStartStopVoiceChannel.TotalDataCnt >> 3) * 6);
        //此处distance*10000会有数据溢出，故仅放大1000倍
        MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = (distance * 512 / 
                                                                                    (EngineStartStopVoiceChannel.TotalDataCnt - ((EngineStartStopVoiceChannel.TotalDataCnt >> 3) * 6))) * 16;                        
                                                                                    
        EngineStartStopVoiceChannel.AmplitudeFactor = (PERCENT_MIXER_BASE - MixerChannel[engine.LowSpeedChannel].AmplitudeFactor);
    }
}


/**********************************引擎熄火处理******************************************/
uint32_t EngineStopHandlerWithoutStopFile()
{
    MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE;
    MixerChannel[engine.HighSpeedChannel].AmplitudeFactor = 0;

    MixerChannel[engine.HighSpeedChannel].SoundData = 0;

    engine.status = ENGINE_STOPING;
    engine.StopTime = 0;
    return 0;
}

void EngineStopingHandlerWithoutStopFile()
{
    static uint32_t StartStopTime = 0;
    uint32_t SpeedTemp;
    AudioFileReadHandler(&MixerChannel[engine.LowSpeedChannel]);
    if(engine.StopTime == 0)
    {
        //播放关闭声音时初次进入
        StartStopTime = GetSystemTime() - 1;
        engine.StopTime  = 1;
    }
    else
    {
        engine.StopTime = GetSystemTime() - StartStopTime;
        if(engine.StopTime < ENGINE_STOP_LAST_TIME)
        {
            //引擎停止声音持续播放中
            //声音逐渐减弱
            //percent = 
            MixerChannel[engine.LowSpeedChannel].AmplitudeFactor = PERCENT_MIXER_BASE - engine.StopTime * PERCENT_MIXER_BASE / ENGINE_STOP_LAST_TIME;
            SpeedTemp = engine.StopTime * ((uint32_t)engine.ForwardSpeedArray[0] * 4 / 5 - ENGINE_STOP_MIN_SPEED);
            SpeedTemp = SpeedTemp / ENGINE_STOP_LAST_TIME;
            SpeedTemp = engine.ForwardSpeedArray[0] * 4 / 5 - SpeedTemp;
            if(engine.LowSpeedChannel == 0)
            {
                MixerChannel[0].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / SpeedTemp;
            }
            else
            {
                MixerChannel[1].TimerInterval = (1000000000 / engine.SampleRate) * (uint32_t)MixerChannel[engine.LowSpeedChannel].OriginSpeed / SpeedTemp;
            }
        }
        else
        {
            //引擎停止声音播放完毕
            engine.status = ENGINE_STOP;
            TryCloseAudioFile(&MixerChannel[0]);
            TryCloseAudioFile(&MixerChannel[1]);
            MixerChannel[0].OriginSpeed = 0;
            MixerChannel[1].OriginSpeed = 0;
            StopMixer();   
        }
    }
    
    
}


uint32_t EngineStopHandlerWithStopFile()
{
    EngineStartStopVoiceChannel.Fp = OpenFile(ENGINE_INDEX(mixer.SoundIndex));
    EngineStartStopVoiceChannel.TotalDataCnt = engine.StopAudioDataCnt;
    EngineStartStopVoiceChannel.AudioStartAddress= engine.StopAudioStartAddr;
    EngineStartStopVoiceChannel.NowBuffFlag = 0;
    EngineStartStopVoiceChannel.ReadFileCnt = 0;
    EngineStartStopVoiceChannel.NowBuffFlag = 0;
    EngineStartStopVoiceChannel.Buff0ReadyFlag = 0;
    EngineStartStopVoiceChannel.Buff1ReadyFlag = 0;
    EngineStartStopVoiceChannel.Buff2ReadyFlag = 0;
    EngineStartStopVoiceChannel.DataPtr = 0;
    EngineStartStopVoiceChannel.ReadFileCnt = 0;
    EngineStartStopVoiceChannel.SoundData = 0;
    EngineStartStopVoiceChannel.AmplitudeFactor = PERCENT_MIXER_BASE;

    //读取引擎关闭的声音数据，由于其他2个通道有3个缓冲，所以
    //时间足够此处读取的
    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    AudioFileReadHandler(&EngineStartStopVoiceChannel);

    engine.status = ENGINE_STOPING;
    MixerChannel[0].AmplitudeFactor = 0;
    MixerChannel[1].AmplitudeFactor = 0;
    //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = (uint32_t)STANDARD_ARR_VALUE;
    EngineStartStopVoiceChannel.TimerInterval = 1000000000 / engine.SampleRate;


    EngineStartStopVoiceChannel.AmplitudeFactor = PERCENT_MIXER_BASE;
    
    MixerChannel[engine.LowSpeedChannel].SoundData = 0;
    MixerChannel[engine.HighSpeedChannel].SoundData = 0;

    engine.status = ENGINE_STOPING;
    engine.StopTime = 0;
    return 0;
}


void EngineStopingHandlerWithStopFile()
{
    AudioFileReadHandler(&EngineStartStopVoiceChannel);
    if(EngineStartStopVoiceChannel.ReadFileCnt == 0)
    {
        engine.status = ENGINE_STOP;
        TryCloseAudioFile(&MixerChannel[0]);
        TryCloseAudioFile(&MixerChannel[1]);
        MixerChannel[0].OriginSpeed = 0;
        MixerChannel[1].OriginSpeed = 0;
        EngineStartStopVoiceChannel.SoundData = 0;
        StopMixer();    

    }
    
}


/******************************引擎状态的状态判断***********************************/
void EngineStatusHandler()
{
     if(engine.status == ENGINE_STOP)
    {
        //引擎当前状态为关闭
        //引擎速度大于静默速度一定值时，引擎启动
        //if(IsTurnOverStart() ==   true && mixer.MixerEnableFlag == ENABLE)
        //上电就启动
        //if(mixer.MixerEnableFlag == ENABLE)
        //if(IsTurnOverStart() ==   true && mixer.MixerEnableFlag == ENABLE)
        if((MotorSpeed.DistSpeed > engine.ForwardLowSpeed || MotorSpeed.EvSpeedLevel != EV_SPEED_HOLD)
            && mixer.MixerEnableFlag != DISABLE)
        {
            mixer.VolumeSlopePercent = CalVolumeSlopePercent(engine.ForwardLowSpeed);
            EngineStartHandler();
        }
    }
    else if(engine.status == ENGINE_STARTING)
    {
        //引擎启动过程
        
        EngineStartingHandler();
        
    }
    else if(engine.status == ENGINE_STOPING)
    {
        //引擎熄火过程
        if(engine.StopAudioDataCnt == 0)
        {
            EngineStopingHandlerWithoutStopFile();
        }
        else
        {
            EngineStopingHandlerWithStopFile();
        }
    }
    else
    {
        SetMixerChannel(MotorSpeed.RealSpeed);
        
        //引擎当前状态为运行状态

           //在怠速区间，一定时候速度没变，则熄火

           
           //暂无熄火

            if(engine.FlameOutSwitch != DISABLE //若设定了不熄火，则直接跳转到else中
              && MotorSpeed.VehicleStatu != BREAK
              //&& IsTurnBelowStop() == true
              && MotorSpeed.RealSpeed <= engine.ForwardLowSpeed
              && MotorSpeed.EvSpeedLevel == EV_SPEED_HOLD)//只有在HOLD时才会熄火
            {
                if(GetSystemTime() - engine.InFlameOutTime >= engine.FlameOutTime * TIMER_MULTI)
                {
                    //油门在一定时间内都在熄火的区间内
                    if(engine.StopAudioDataCnt == 0)
                    { 
                        EngineStopHandlerWithoutStopFile();
                    }
                    else
                    {
                        EngineStopHandlerWithStopFile();
                    }
                }
            }
            else
            {
                engine.InFlameOutTime = GetSystemTime();
                //MotorSpeed.SlientSpeed = MotorSpeed.RealSpeed;
            }


            //引擎正常工作，读取两个通道的语音数据
            if(MixerChannel[0].AmplitudeFactor != 0)
                AudioFileReadHandler(&MixerChannel[0]);
            if(MixerChannel[1].AmplitudeFactor != 0)
                AudioFileReadHandler(&MixerChannel[1]);
    }
}


void EngineHandler()
{
    CalSpeedAndBrakeAndGearBoxHandler();
    EngineStatusHandler();
}




//uint32_t TimerCnt[2];
//uint32_t CCDistance[4];

///**************************引擎混音通道的中断处理***********************************/
//void MixerChannel0IntHandler()
//{
//    
//    TIMER_MIXER_CHANNEL0->TASKS_CAPTURE[3] = 1;
//    CCDistance[0] = TIMER_MIXER_CHANNEL0->CC[3];
//    //CCDistance[0] = TIMER_MIXER_CHANNEL0->CC[1] - TIMER_MIXER_CHANNEL0->CC[0];
//    //MIXER_CHANNEL0_TIMER.p_reg->CC[0] = MIXER_CHANNEL0_TIMER.p_reg->CC[0] + MixerChannel[0].TimerInterval;
//    TIMER_MIXER_CHANNEL0->EVENTS_COMPARE[0] = 0;
//    TIMER_MIXER_CHANNEL0->EVENTS_COMPARE[1] = 0;
//    TIMER_MIXER_CHANNEL0->EVENTS_COMPARE[2] = 0;
//    
//    TIMER_MIXER_CHANNEL0->CC[0] = MixerChannel[0].TimerInterval;
//    //nrf_timer_event_clear(TIMER_MIXER_CHANNEL0, NRF_TIMER_EVENT_COMPARE0);
//    
//    AudioChannelGetDataInTimeInterruptWithSecure(&MixerChannel[0]);

//    AudioOutDac();
//    TimerCnt[0]++;
//    
//}


//void MixerChannel1IntHandler()
//{
//    TIMER_MIXER_CHANNEL1->TASKS_CAPTURE[3] = 1;
//    CCDistance[1] = TIMER_MIXER_CHANNEL1->CC[3];
//    TIMER_MIXER_CHANNEL1->EVENTS_COMPARE[0] = 0;
//    TIMER_MIXER_CHANNEL1->EVENTS_COMPARE[1] = 0;
//    TIMER_MIXER_CHANNEL1->EVENTS_COMPARE[2] = 0;
//    //nrf_timer_event_clear(TIMER_MIXER_CHANNEL1, NRF_TIMER_EVENT_COMPARE0);
//    if(engine.status == ENGINE_RUNNING || (engine.status == ENGINE_STOPING && engine.StopAudioDataCnt == 0))
//    {
//        TIMER_MIXER_CHANNEL1->CC[0] = MixerChannel[1].TimerInterval;
//        //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = MIXER_CHANNEL1_TIMER.p_reg->CC[0] + MixerChannel[1].TimerInterval;
//        //引擎运行时，或者播放熄火声音，熄火声音由怠速合成
//        AudioChannelGetDataInTimeInterruptWithSecure(&MixerChannel[1]);
//    }
//    else if(engine.status == ENGINE_STARTING || (engine.status == ENGINE_STOPING && engine.StopAudioDataCnt != 0))
//    {
//        TIMER_MIXER_CHANNEL1->CC[0] = EngineStartStopVoiceChannel.TimerInterval;
//        //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = MIXER_CHANNEL1_TIMER.p_reg->CC[0] + EngineStartStopVoiceChannel.TimerInterval;
//        //引擎运行时，或者播放熄火声音，熄火声音有独立语音
//        AudioChannelGetDataInTimeInterruptWithSecure(&EngineStartStopVoiceChannel);
//    }
//    else
//    {
//        TIMER_MIXER_CHANNEL1->CC[0] = STANDARD_ARR_VALUE;
//        //MIXER_CHANNEL1_TIMER.p_reg->CC[0] = MIXER_CHANNEL1_TIMER.p_reg->CC[0] + STANDARD_ARR_VALUE;
//        MixerChannel[1].SoundData = 0;
//        EngineStartStopVoiceChannel.SoundData = 0;
//    }

//    AudioOutDac();
//    TimerCnt[1]++;

//    //TimerCC[TimerPtr] = MIXER_CHANNEL1_TIMER.p_reg->CC[0];
//    //TimerPtr++;
//}


//int32_t GetEngineAudioData()
//{
//    int32_t data;
//    if(engine.status == ENGINE_RUNNING || (engine.status == ENGINE_STOPING && engine.StopAudioDataCnt == 0))
//    {
//        //播发发动机运行声音，或者熄火声音，若没有熄火声音，熄火声音通过怠速合成
//        data = (int32_t)MixerChannel[0].SoundData + (int32_t)MixerChannel[1].SoundData;
//    }
//    else if(engine.status == ENGINE_STOP)
//    {
//        data = 0;
//    }
//    else
//    {
//        data = (int32_t)EngineStartStopVoiceChannel.SoundData + (int32_t)MixerChannel[0].SoundData;
//    }
//    data = data * mixer.VolumeSlopePercent / 10000;
//    return data;
//}

//void EnableMixerChannelTimer()
//{
//    nrf_drv_timer_enable(&MIXER_CHANNEL0_TIMER);    
//    nrf_drv_timer_enable(&MIXER_CHANNEL1_TIMER);    
//}

//void DisableMixerChannelTimer()
//{
//    nrf_drv_timer_disable(&MIXER_CHANNEL0_TIMER);    
//    nrf_drv_timer_disable(&MIXER_CHANNEL1_TIMER);    
//}


