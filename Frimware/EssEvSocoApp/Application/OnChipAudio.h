#ifndef __ON_CHIP_AUIOD__
#define __ON_CHIP_AUIOD__


#define MENU_SHORT_DI_CNT				4
#define MENU_LONG_DI_CNT				12

struct OnChipPlayInfo
{
    s16 AudioData;
    u32 addr;
    u32 DataCnt;
    u32 PlayedAudioDataCnt;//已播放的数据
    u8 OneDataCnt;//由于片内语音为8k，实际输出频率为44.1K，用该变量记录同一个数据送出去几次，一个数据总共输出5次
    u8 OnePlayCnt;//片内语音连续播放的次数,用于控制声音的长短
    u8 OnePlayCntBak;
    u16 IntervalPlayCnt;//片内语音间隔播放的次数
    s16 MaxAudioData;
    u16 Multi;
    u16 PlayInterval;
};



s32 GetOnChipAudioData(void);
void PlaySystemStartBeep(unsigned char EngineIndex);
u32 OnChipAudioPlay(u8 OnePlayCnt, u16 IntervalPlayCnt, u16 Interval);
u16 CheckOnChipAudioIsPlaying(void);
s32 GetCurrentOnChipAudioData(void);

#endif


