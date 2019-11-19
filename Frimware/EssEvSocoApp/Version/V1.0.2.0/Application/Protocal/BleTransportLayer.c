#include "common.h"
#include <stdint.h>

#include "BleComHw.h"
#include "BleTransportLayer.h"
#include "BleDataLayer.h"

/***Com传输层处理***/
#define FRAME_START			0xFA
#define FRAME_END			0xFD
#define FRAME_TRANSFER		0xFE

struct BleTransportInfo BleTransportLay;

void OneFrameCheckAndHandle()
{
    int i;
    unsigned char crc;
    
    
    if(BleTransportLay.BuffPtr < 2)
        return;

    crc = 0;   
    for(i = 0; i < BleTransportLay.BuffPtr; i++)
    {
        crc ^= BleTransportLay.RxBuff[i];
    }

    if(crc != 0)
        return;


    BleAnalysisMessageId(BleTransportLay.RxBuff, BleTransportLay.BuffPtr - 1);
    //AnalysisApplicationMessageid(TransportLay.MessageId, TransportLay.RxBuff + 1, TransportLay.BuffPtr - 4, SequenceNumber);
}

void BleAnalyzeTransportLayer()
{
    static FunctionalState TransferFlag = DISABLE;
    unsigned char data;

    while(BleCom.RxHead != BleCom.RxTail)
    {
        data = BleCom.RxBuff[BleCom.RxHead];
        switch(BleTransportLay.TransportStage)
        {
            case CONTENT:
                if(TransferFlag == DISABLE)
                {
                    if(data == FRAME_START)
                    {
                        //在内容里面发现起始位，则认为前一帧消息没有收到结束位，重新开始内容的记录
                        BleTransportLay.BuffPtr = 0;
                    }
                    else if(data == FRAME_END)
                    {
                        OneFrameCheckAndHandle();
                        BleTransportLay.TransportStage = START;
                    }
                    else if(data == FRAME_TRANSFER)
                    {
                        TransferFlag = ENABLE;
                    }
                    else
                    {
                        BleTransportLay.RxBuff[BleTransportLay.BuffPtr] = data;
                        BleTransportLay.BuffPtr++;
                        if(BleTransportLay.BuffPtr >= TRANSPORT_RX_BUFF_SIZE)
                        {
                            //内容长度超过正常缓冲的长度，必然是同时遗漏了起始位和结束位
                            BleTransportLay.TransportStage = START;
                        }
                    }
                }
                else
                {
                    if(data == FRAME_START || data == FRAME_END || data == FRAME_TRANSFER)
                    {
                        BleTransportLay.RxBuff[BleTransportLay.BuffPtr] = data;
                        BleTransportLay.BuffPtr++;
                        if(BleTransportLay.BuffPtr >= TRANSPORT_RX_BUFF_SIZE)
                        {
                            //内容长度超过正常缓冲的长度，必然是同时遗漏了起始位和结束位
                            BleTransportLay.TransportStage = START;
                        }
                    }
                    else
                    {
                        //转移字符后面的字符不是有效字符，此帧错误
                        BleTransportLay.TransportStage = START;
                    }
                    TransferFlag = DISABLE;
                }
                break;
            case START:
                if(TransferFlag == DISABLE)
                {
                    if(data == FRAME_START)
                    {
                         BleTransportLay.TransportStage = CONTENT;
                         BleTransportLay.BuffPtr = 0;
                    }
                    else if(data == FRAME_TRANSFER)
                    {
                        TransferFlag = ENABLE;
                    }
                }
                else
                {
                    TransferFlag = DISABLE;
                }
                break;
            default:
                break;
        }
        BleCom.RxHead++;
        if(BleCom.RxHead >= BLE_COM_RX_SIZE)
        {
            BleCom.RxHead = 0;
        }
    }
}


void BleSendOneFrame(unsigned char *buff, int cnt)
{
    int32_t i;
    int32_t TransFormCnt = 0;
    int32_t SendCnt;
    uint8_t Crc;
    uint8_t data;
    uint8_t *BuffPtr;

    //StartTime = Clock_getTicks();
    Crc = 0;
    for(i = 0; i < cnt; i++)
    {
        Crc ^= buff[i];
    }

    BuffPtr = BleTransportLay.TxBuff;
    *BuffPtr++ = 0xFA;
    for(i = 0; i < cnt; i++)
    {
        data = buff[i];
        if(data == 0xFA || data == 0xFE || data == 0xFD)
        {
            *BuffPtr++ = 0xFE;
            TransFormCnt++;
        }
        *BuffPtr++ = data;
    }

    if(Crc == 0xFA || Crc == 0xFE || Crc == 0xFD)
    {
        *BuffPtr++ = 0xFE;
        TransFormCnt++;
    }
    *BuffPtr++ = Crc;
    *BuffPtr = 0xFD;

    //总共发送的数据量为数据内容+ 转义字符数量+ 3个字节(起始、结束、CRC)
    SendCnt = cnt + TransFormCnt + 3;
    BleComSendData(BleTransportLay.TxBuff, SendCnt);
}




