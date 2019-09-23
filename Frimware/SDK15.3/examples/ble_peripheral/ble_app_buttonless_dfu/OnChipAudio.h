#ifndef __ON_CHIP_AUIOD__
#define __ON_CHIP_AUIOD__


#define MENU_SHORT_DI_CNT				4
#define MENU_LONG_DI_CNT				12

struct OnChipPlayInfo
{
    int16_t AudioData;
    uint32_t addr;
    uint32_t DataCnt;
    uint32_t PlayedAudioDataCnt;//已播放的数据
    uint8_t OneDataCnt;//由于片内语音为8k，实际输出频率为44.1K，用该变量记录同一个数据送出去几次，一个数据总共输出5次
    uint8_t OnePlayCnt;//片内语音连续播放的次数,用于控制声音的长短
    uint8_t OnePlayCntBak;
    uint16_t IntervalPlayCnt;//片内语音间隔播放的次数
    int16_t MaxAudioData;
    uint16_t Multi;
    uint16_t PlayInterval;
};



int32_t GetOnChipAudioData(void);
void PlaySystemStartBeep(unsigned char EngineIndex);
uint32_t OnChipAudioPlay(uint8_t OnePlayCnt, uint16_t IntervalPlayCnt, uint16_t Interval);
uint16_t CheckOnChipAudioIsPlaying(void);
int32_t GetCurrentOnChipAudioData(void);

#endif


