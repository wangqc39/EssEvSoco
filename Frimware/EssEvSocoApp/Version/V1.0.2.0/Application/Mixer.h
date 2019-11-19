#ifndef __MIXER__
#define __MIXER__

#include "SystemHw.h"
#include "fs.h"

typedef enum {CHANNEL_CLOSE = 0, CHANNEL_OPEN = 1} ChannnelStatusFlag;



#define AUDIO_DATA_READ_CNT			180//600//210//159//能被3整除


#define AUDIO_RATE				22050//44100
#define SAMPLE_CNT_1MS			(AUDIO_RATE / 1000)
#define STANDARD_ARR_VALUE		(SYSTEM_CLK / AUDIO_RATE)


#define DAC_OUT_RATE            (22050)
#define DAC_OUT_ARR_VALUE		(SYSTEM_CLK / DAC_OUT_RATE)


#define PERCENT_MIXER_BASE				8192
#define PERCENT_MIXER_BASE_SHIFT		13

//音量大小定义
#define MIN_VOLUME			0
#define MAX_VOLUME			8
#define DEFAULT_SOUND_LEVEL		2
				

/*
//支持可切换的引擎数量
#define MAX_ENGINE_NUM					2
#define MAX_SOUND_NUM			MAX_ENGINE_NUM
*/

struct MixerChannelInfo
{
    struct FileInfo *Fp;
    s16 SoundData;

    
    s32 AmplitudeFactor;//数值为0-10000
    u16 OriginSpeed;
    u32 TotalDataCnt;//语音文件中，数据的个数
    u8 NowBuffFlag;
    u8 Buff0ReadyFlag;
    u8 Buff1ReadyFlag;
    u8 Buff2ReadyFlag;

    u8 buff0[AUDIO_DATA_READ_CNT];
    u8 buff1[AUDIO_DATA_READ_CNT];
    u8 buff2[AUDIO_DATA_READ_CNT];
    u16 Buff0DataCnt;
    u16 Buff1DataCnt;
    u16 Buff2DataCnt;
    u32 ReadFileCnt;
    u32 DataPtr;
    //u16 AudioStartSector;
    u32 AudioStartAddress;
    ChannnelStatusFlag ChannelStatus;
    u8 id;

    u8 PlayCnt;//用于泄压阀声音播放，增加时值为0XFF，表示一直播放，其余表示播放的次数

    u32 CycleCnt;
};



struct MixerInfo
{
    FunctionalState MixerEnableFlag;
    unsigned char SoundIndex;//该值需要时刻与flash中的系统配置表中的值保持一直
    FunctionalState AmplifierEnable;
    u8 VolumeLevel;
    u8 VolumeSlope;//音量斜率,范围0-10, 0:斜率为0，对原始音源不做调整，10：最大调整，具体和PERCENT_STEP_BELOW_MIDDLE相关
    u16 VolumeSlopePercent;//根据VolumeSlope和当前的RealSpeed，计算出Percent，最终在引擎声音数据上进行缩放
    bool Slinet;
};


extern struct MixerInfo mixer;
extern uint8_t RealVolume;


int SoundIndexSetBySystemConfig(unsigned char *SoundIndex);
void StopMixer(void);
void StartMixer(void);
void AudioFileReadHandler(struct MixerChannelInfo *ThisChannel);
void AudioOutTimerIntHandler(void);
void AudioChannelGetDataInTimeInterruptWithSecure(struct MixerChannelInfo *ThisChannel);
void AudioOutHwConfig(void);
//int TryChangeSoundIndex(unsigned char DistIndex);
void ChangeSoundIndex(void);

int VolumeSetBySystemConfig(unsigned char *VolumeLevel);
void AddVolume(void);
void MinusVolume(void);
void AmplifierEnable(void);
void AmplifierControlHandler(void);
void HighVoltageMixerHandler(void);
void MixerBleTopLevelHandler(void);
int32_t TryChangeVehileRollBack(unsigned char DistIndex, bool IsStartUp);
int32_t TryChangeVehileIndexLoop(unsigned char DistIndex, bool IsStartUp);
void VolumeHandler(void);
void CalRealVolume(void);
void AudioOutDac(void);





#endif


