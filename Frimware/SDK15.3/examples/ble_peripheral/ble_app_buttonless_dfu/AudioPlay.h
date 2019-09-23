#ifndef __AUDIO_PLAY__
#define __AUDIO_PLAY__

#define AUDIO_OUT_BUFF_SIZE              28   //IS2 DMA缓冲的数量 (1000000 / 44100) * AUDIO_OUT_BUFF_SIZE为一个缓冲播放的时长616us

#define AUDIO_OUT_FREQUENCY                44100
#define AUDIO_OUT_INTERVAL                  (1000000000 / 44100)//每个输出音频数据之间的时间间隔，单位ns

/** 
 * [AudioNode description]单个音频采样点的节点，包含音频数据和时间点信息，时间点的单位为ns
 * @Author   tin39
 * @DateTime 2019年7月23日T9:30:14+0800
 * @return                            [description]
 */
struct AudioNodeInfo
{
    int32_t data;
    uint64_t time;
};

struct AudioOutInfo
{
    //两个混合通道数据采样节点和节点数量
    struct AudioNodeInfo AudioNode[2][AUDIO_OUT_BUFF_SIZE * 3];//每个采样点的音频节点，包含音频数据和时间点
    int16_t AudioNodeCnt[2];

    int32_t I2sBuff[2][AUDIO_OUT_BUFF_SIZE];
    uint64_t timeStamp[AUDIO_OUT_BUFF_SIZE];
    int32_t *I2sSendingBuffPtr;
    int32_t *I2sFillingBuffPtr;
};

extern struct AudioOutInfo AudioOut;

void SetI2sData(uint16_t AudioData);
void AudioOutVarInit(void);
void I2sInit(void);
void I2sIntRestart(void);
void I2sIntStop(void);
void ClearAudioPlayBuff(void);



#endif
