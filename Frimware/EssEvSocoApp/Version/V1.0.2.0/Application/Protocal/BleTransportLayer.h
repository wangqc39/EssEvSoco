#ifndef __BLE_TRANSPORT_LAYER__
#define __BLE_TRANSPORT_LAYER__


#define TRANSPORT_RX_BUFF_SIZE		25
#define TRANSPORT_TX_BUFF_SIZE		50
typedef enum {START = 0, CONTENT = 1}TransportStageType;
struct BleTransportInfo
{
    TransportStageType TransportStage;
    unsigned char BuffPtr;
    unsigned char RxBuff[TRANSPORT_RX_BUFF_SIZE];
    unsigned char TxBuff[TRANSPORT_TX_BUFF_SIZE];
};



void BleAnalyzeTransportLayer(void);
void BleSendOneFrame(unsigned char *buff, int cnt);


#endif

