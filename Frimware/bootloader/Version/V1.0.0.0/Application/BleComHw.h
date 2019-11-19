#ifndef __BLE_COM_HW__
#define __BLE_COM_HW__

//数据物理层
#define BLE_COM_RX_SIZE			400
#define BLE_COM_TX_SIZE			1000
struct BleComInfo
{
    unsigned short int TxHead;
    unsigned short int TxTail;
    unsigned short int RxHead;
    unsigned short int RxTail;
    FunctionalState IsTxing;
    unsigned char RxBuff[BLE_COM_RX_SIZE];  //接收数据缓冲，循环队列
    unsigned char TxBuff[BLE_COM_TX_SIZE];
};


extern struct BleComInfo BleCom;

void BleComHwInit(void);
void BleComIntHandler(void); 
void BleComSendData(unsigned char *buff, unsigned short int cnt);

#endif

