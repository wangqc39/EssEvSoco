#ifndef __BLE_SERVICE__
#define __BLE_SERVICE__





void BleProtocalServiceInit(void);
void BleSendOneFrame(unsigned char *buff, uint16_t cnt);
void BleRxDataHandler(void);
void AddNodeToBleRequestCmdTable(uint8_t *data, uint8_t length);

void BleRequestCmdTableHandler(void);



#endif
