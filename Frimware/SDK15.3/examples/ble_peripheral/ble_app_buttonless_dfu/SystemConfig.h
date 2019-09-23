#ifndef __SYSTEM_CONFIG__
#define __SYSTEM_CONFIG__



                        
void ReadSystemConfig(unsigned int offset, unsigned char *buff, unsigned int length);
int WriteSystemConfig(unsigned int offset, unsigned char *buff, unsigned int length);
void AnalyzeSystemConfig(void);
void WriteSoundIndexConfig(unsigned char SoundIndex);
void WriteVolumeConfig(unsigned char VolumeLevel);
void WirteRcCaptureConfig(uint16_t ForwardHighPwm, uint16_t BackwardHighPwm, uint16_t MiddleThrottlePwm);


int32_t GetOneSystemParam(uint8_t *buff, uint8_t Index);
int32_t SetOneSystemParam(uint8_t *SetValuePtr, uint8_t *ParamValuePtr, uint8_t length, uint8_t Index);
int32_t WriteSystemParamTable(void);
void AnalyzeDeviceConfig(void);


#endif

