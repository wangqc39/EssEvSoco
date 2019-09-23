#ifndef __PARAM__
#define __PARAM__

int32_t CheckParamIndex(unsigned char *ParamAddr, int ParamLength, int CheckLength, unsigned int DefaultValue, unsigned int Min, unsigned int Max);
unsigned int CalSum(unsigned int *StartAddr, unsigned int IntLength);
void WriteOneParamToTable(unsigned char *SourceAddr, int offset, int Length, unsigned char *DistTable);
int32_t InitOneParam(uint8_t *ConfigBuff, uint8_t *Param, int32_t offset, int32_t length, uint32_t DefaultValue, uint32_t Min, uint32_t Max);
int32_t InitBuffParam(uint8_t *ConfigBuff, uint8_t *Param, int32_t offset, int32_t length, uint8_t DefaultValue, uint8_t Min, uint8_t Max);



#endif

