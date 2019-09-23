#include "common.h"
#include "OnChipAudio.h"
#include "mixer.h"
#include "ActionTick.h"
#include "SystemInfo.h"

struct OnChipPlayInfo OnChipPlayer;


//片内语音的起始地址，从48.5K处开始
#define OnChipAudioStartAddr				(OnChipAudioMenuAddr + 0x200)   //

#define AUDIO_DI			0

#define ON_CHIP_ONE_PLAY_CNT            5


//0未在播放
uint16_t CheckOnChipAudioIsPlaying()
{
    return OnChipPlayer.IntervalPlayCnt;
}

//Interva单位ms
uint32_t OnChipAudioPlay(uint8_t OnePlayCnt, uint16_t IntervalPlayCnt, uint16_t Interval)
{
    if(CheckOnChipAudioIsPlaying() != 0)
        return 4;

    OnChipPlayer.addr = OnChipAudioMenuAddr + 512;//*(uint32_t *)(OnChipAudioMenuAddr);
    OnChipPlayer.DataCnt = *(uint32_t *)(OnChipAudioMenuAddr + 4);
    

    //检查语音地址是否小于最小地址
    if(OnChipPlayer.addr < OnChipAudioStartAddr)
        return 1;
    //检查语音终止地址是否超出片内语音地址范围
    if((OnChipPlayer.addr + OnChipPlayer.DataCnt) >= SYSTEM_INFO_ADDR)
        return 2;


    //对语音文件大小进行检查，超过0xffff，无法使用DMA进行播放
    if(OnChipPlayer.DataCnt > 0xffff)
        return 3;

    OnChipPlayer.OnePlayCnt = OnePlayCnt;
    OnChipPlayer.OnePlayCntBak = OnePlayCnt;
    OnChipPlayer.IntervalPlayCnt = IntervalPlayCnt;
    OnChipPlayer.OneDataCnt = ON_CHIP_ONE_PLAY_CNT;
    OnChipPlayer.PlayedAudioDataCnt = 0;
    OnChipPlayer.AudioData = *(uint16_t *)OnChipPlayer.addr + 0x8000;
    OnChipPlayer.PlayInterval = Interval * 10;

    return 0;
}

int32_t GetOnChipAudioData()
{
    static uint32_t NextPlayTime = 0;
    //无片内语音播放的情况
    if(CheckOnChipAudioIsPlaying() == 0)
    {
        OnChipPlayer.AudioData = 0;
        return OnChipPlayer.AudioData;
    }

    if(GetSystemTime() < NextPlayTime)
    {
        //若未到播放时间，则返回，用于有间隔的播放滴滴声
        OnChipPlayer.AudioData = 0;
        return OnChipPlayer.AudioData;
    }

    if(OnChipPlayer.OneDataCnt > 0)
    {
        OnChipPlayer.OneDataCnt--;
        return OnChipPlayer.AudioData;
    }
    else
    {
        OnChipPlayer.PlayedAudioDataCnt += 2;
        if(OnChipPlayer.PlayedAudioDataCnt >= OnChipPlayer.DataCnt)
        {
            OnChipPlayer.PlayedAudioDataCnt = 0;
            OnChipPlayer.OnePlayCnt--;
            if(OnChipPlayer.OnePlayCnt == 0)
            {
                //一次播放完成
                OnChipPlayer.OnePlayCnt = OnChipPlayer.OnePlayCntBak;
                NextPlayTime = OnChipPlayer.PlayInterval + GetSystemTime();
                OnChipPlayer.IntervalPlayCnt--;
            }
            
        }
        
        OnChipPlayer.AudioData = *(uint16_t *)(OnChipPlayer.addr + OnChipPlayer.PlayedAudioDataCnt)
                                                  + 0x8000;
        OnChipPlayer.OneDataCnt = ON_CHIP_ONE_PLAY_CNT;    
        return OnChipPlayer.AudioData;
    }
}

int32_t GetCurrentOnChipAudioData()
{
    return OnChipPlayer.AudioData;
}


void PlaySystemStartBeep(unsigned char EngineIndex)
{
    OnChipAudioPlay(2, EngineIndex + 1, 100);
}
