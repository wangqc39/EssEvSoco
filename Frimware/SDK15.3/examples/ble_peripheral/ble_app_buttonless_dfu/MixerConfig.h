#ifndef __MIXER_CONFIG__
#define __MIXER_CONFIG__

void AnalyzeMixerConfig(unsigned char VoiceIndex);
void InitialMixerConfigTable(void);
int WriteSoundConfig(unsigned char SoundIndex, unsigned int offset, unsigned char *buff, unsigned int length);
void ReadSoundConfig(unsigned char SoundIndex, unsigned int offset, unsigned char *buff, unsigned int length);


int32_t GetOneVehicleParam(uint8_t *buff, uint8_t Index);
int32_t SetOneVehicleParam(uint8_t *SetValuePtr, uint8_t *ParamValuePtr, uint8_t length, uint8_t Index);
int32_t WriteVehicleParamTable(uint8_t VehicleIndex);

#endif


