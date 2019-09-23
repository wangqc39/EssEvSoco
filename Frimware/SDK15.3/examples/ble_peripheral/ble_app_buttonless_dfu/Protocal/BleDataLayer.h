#ifndef __BLE_DATA_LAYER__
#define __BLE_DATA_LAYER__


void BleAnalysisMessageId(uint8_t *buff, uint32_t length);
int32_t GetNextSoundBlockIndex(void);
int32_t WriteSoundBlockDataCmdHandler(uint8_t *buff, uint32_t length);
void AddNodeToBleRequestCmdTable(uint8_t *data, uint8_t length);



#endif

