#ifndef __ENGINE__
#define __ENGINE__

#include <stdint.h>


typedef enum {ENGINE_STOP = 0, ENGINE_RUNNING = 1, 
                         ENGINE_STARTING = 2, ENGINE_STOPING = 3} EngineStatusFlag;

//#define MIXER_CHANNEL0_TIMER			TIM6
//#define MIXER_CHANNEL1_TIMER			TIM7

                         
#define MAX_AUDIO_FILE_NUM_ONE_DIRECT				20
#define BACKWARD_AUDIO_CNT							5

//混音通道数
#define ENGINE_MIXER_CHANNEL_NUM		2

//一段语音头部所占扇区
#define ONE_AUDIO_HEAD_SECTOR_CNT		4
//每段语音的头部信息偏移量，总大小2048字节
#define FORWARD_AUDIO_CNT_OFFSET		20
#define BACKWARD_AUDIO_CNT_OFFSET	21
#define SPEED_TABLE_OFFSET		22
#define AUDIO_LENGTH_OFFSET	102
#define AUDIO_START_SECTOR_OFFSET		262
#define ACCELERATE_TABLE_OFFSET		354
#define OVERLOAD_FLAG_OFFSET			394
//#define OVERLOAD_DELAY_TIME_OFFSET	395
//#define AUDIO_DATA_SECRET_OFFSET		397
#define START_SOUND_LENGTH_OFFSET			342
#define START_SOUND_START_SECTOR_OFFSET		346
#define STOP_SOUND_LENGTH_OFFSET				348
#define STOP_SOUND_START_SECTOR_OFFSET		352
#define AUDIO_INFO_GEARBOX_OFFSET			396
#define ENGINE_DECELERATE_INTERVAL_OFFSET			428
//#define BREAK_SOUND_LENGTH_OFFSET			430
//#define BREAK_SOUND_START_SECTOR				434

#define AUDIO_SAMPLE_RATE_OFFSET            450
#define AUDIO_SAMPLE_RATE_LENGTH            4
#define AUDIO_BIT_RATE_OFFSET               (AUDIO_SAMPLE_RATE_OFFSET + AUDIO_SAMPLE_RATE_LENGTH)
#define AUDIO_BIT_RATE_LENGTH               2


struct EngineInfo
{
    uint32_t SampleRate;//Mp3采样率
    //uint16_t BitRate;//Mp3码流,实际并没有作用
    EngineStatusFlag status;

    

    FunctionalState FlameOutSwitch;
    uint16_t FlameOutTime;//当速度处于怠速区间，并且MaxSlientTime内没有变化，则熄火
    uint32_t InFlameOutTime;
    uint32_t StopTime;//引擎关闭已进行的时间

    
    uint16_t ThrottleUpResponse;//油门响应，值越大，响应越慢,单位100ns
    uint16_t ThrottleDownResponse;
    uint16_t ThrottleDownResponseOri;//配置值，与EngineDecResponse一起计算得到ThrottleDownResponse

    

    int16_t SpeedStage;//标识当前速度所在的速度阶段，有叠加时，算高的那段速度区间
    int16_t UnMixSpeed;//从该值到标准速度之间的区间，不需要混音，当速度阶段为0时，该值无效
    uint8_t ChannelNum;//通道的数量，后续用来判断是否需要混音
    uint8_t LowSpeedChannel;
    uint8_t HighSpeedChannel;

    FunctionalState OverloadExistFlag;
    FunctionalState OverloadEnableFlag;
    uint16_t OverloadDelayTime;

    //int16_t AccelerateArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    int16_t AccelerateArrayOri[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    uint16_t EngineAccResponseP;
    uint16_t EngineAccResponseD;
    uint16_t EngineDecResponse;


    int16_t ForwardLowSpeed;//前进音源的低速
    int16_t ForwardHighSpeed;//前进音源的高速
    int16_t BackwardLowSpeed;//前进音源的低速
    int16_t BackwardHighSpeed;//前进音源的高速

    int16_t ForwardSpeedArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    uint32_t ForwardDataCntArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//语音段中语音的个数
    int16_t BackwardSpeedArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    //uint32_t BackwardDataCntArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//语音段中语音的个数
    uint32_t ForwardAudioStartAddrArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    //uint32_t BackwardAudioStartAddrArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];
    //int16_t GearBoxAccelerateArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];

    uint8_t ForwardAudioFileNum;
    uint8_t BackwardAudioFileNum;

    uint32_t StartAudioDataCnt;
    uint32_t StartAudioStartAddr;
    uint32_t StopAudioDataCnt;
    uint32_t StopAudioStartAddr;

    uint32_t MaxSpeedLimit;//根据电动车档位不同，最大速度也会不同
};

extern struct EngineInfo engine;
extern struct MixerChannelInfo MixerChannel[ENGINE_MIXER_CHANNEL_NUM];
extern struct MixerChannelInfo EngineStartStopVoiceChannel;


int EngineSetByMixerConfig(uint32_t *FlameOutTime, uint16_t *ThrottleDownResponse, uint16_t *ThrottleUpResponse, FunctionalState *OverloadExistFlag,
                                                            FunctionalState *OverloadEnableFlag, uint16_t *OverloadDelayTime, int16_t *AccelerateArray, FunctionalState *EngineResponseEnable,
                                                            uint16_t *EngineResponse);
//void MixerChannel0IntHandler(void);
//void MixerChannel1IntHandler(void);
int32_t GetEngineAudioData(void);
void EngineHandler(void);
void EngineMixerChannelInit(void);
int AnalyzeEngineFile(unsigned char EngineIndex);
void CalDecelerartion(void);
void CalAccelerateArray(int16_t *AccelerationArray, uint16_t factor);
uint32_t EngineStartHandler(void);
void EnableMixerChannelTimer(void);
void DisableMixerChannelTimer(void);
void TryCloseAudioFile(struct MixerChannelInfo *Channel);





#endif


