#include <stdio.h>
#include <string.h>

//#include <xdc/std.h>
//#include <ti/sysbios/BIOS.h>
//#include <ti/sysbios/knl/Semaphore.h>
//#include <ti/sysbios/knl/Queue.h>
//#include <ti/sysbios/knl/Task.h>
//#include <ti/sysbios/knl/Clock.h>
//#include <ti/sysbios/knl/Event.h>

#include "board.h"
#include <ti/drivers/uart/UARTCC26XX.h>
#include "uart.h"

#include "hci_tl.h"

#include "simple_gatt_profile.h"


#define SIZE_RX_BUFF			1000

#define CNT_SEND_CMD_POOL	21

#define FRAME_START 			0xFA
#define FRAME_END			0xFD
#define FRAME_TRANSFER		0xFE


struct SendCmdPoolInfo
{
    uint8_t Buff[CNT_SEND_CMD_POOL];
    uint8_t Cnt;
};


#define CNT_NOOD_BLE_NOTIFY_POOL        50
struct BleNotifyPoolNoodInfo
{
    uint8_t Buff[20];
    uint8_t Cnt;
};

struct BleNotifyPoolInfo
{
    struct BleNotifyPoolNoodInfo Node[CNT_NOOD_BLE_NOTIFY_POOL];
    uint8_t Head, Tail;
};

struct BleNotifyPoolInfo BleNotifyPool;



UART_Handle UARTHandle;
UART_Params UARTparams;
uint8_t SendBuff[50];
uint8_t ReadBuff[100];
uint8_t RxBuff[SIZE_RX_BUFF];


//发送命令池，瞬时内BLE收到数据可能快于串口发送数据，会导致数据丢失，所以通过命令池的方式防止数据丢失
struct SendCmdPoolInfo SendCmdPool[CNT_SEND_CMD_POOL];
uint8_t SendCmdPoolHead, SendCmdPoolTail;

bool UartSendFlag;



uint32_t RxBuffHead, RxBuffTail;


void Uart_ReadCallback(UART_Handle handle, void *rxBuf, size_t size);
void UartSendOnePacket(uint8_t *buff, uint32_t cnt);
void Uart_WriteCallback(UART_Handle handle, void *txBuf, size_t size);


void UartHwInit()
{
    UART_init();                                      //初始化模块的串口功能
    UART_Params_init(&UARTparams);                    //初始化串口参数
    UARTparams.baudRate = 115200;                     //串口波特率115200
    UARTparams.dataLength = UART_LEN_8;               //串口数据位8
    UARTparams.stopBits = UART_STOP_ONE;              //串口停止位1
    UARTparams.readDataMode = UART_DATA_BINARY;       //串口接收数据不做处理
    UARTparams.writeDataMode = UART_DATA_BINARY;        //串口发送数据不做处理
    UARTparams.readMode = UART_MODE_CALLBACK;         //串口异步读
    UARTparams.writeMode = UART_MODE_CALLBACK;        //串口异步写
    UARTparams.readEcho = UART_ECHO_OFF;              //串口不回显
    UARTparams.readReturnMode = UART_RETURN_FULL;  //当接收到换行符时，回调
    UARTparams.readCallback = Uart_ReadCallback;      //串口读回调
    UARTparams.writeCallback = Uart_WriteCallback;    //串口写回调
    UARTparams.readTimeout   = 10000 / Clock_tickPeriod;
    
    UARTHandle = UART_open(Board_UART0, &UARTparams); //打开串口通道
    UART_control(UARTHandle, UARTCC26XX_RETURN_PARTIAL_ENABLE,  NULL);   //允许接收部分回调
    
    UART_read(UARTHandle, ReadBuff, 100);       //打开一个串口读
}

uint32_t StartTime;
uint32_t StopTime;
uint32_t UsedTime;
void UartSendPacket(uint8_t *buff, uint32_t cnt)
{
    //将发送的数据放到发送池中
    SendCmdPool[SendCmdPoolTail].Cnt = cnt;
    memcpy(SendCmdPool[SendCmdPoolTail].Buff, buff, cnt);
    SendCmdPoolTail++;
    if(SendCmdPoolTail >= CNT_SEND_CMD_POOL)
        SendCmdPoolTail = 0;

    //确认串口是否正在发送数据
    if(UartSendFlag == false)
    {
        //串口没有发送数据，进行数据发送
        UartSendFlag = true;
        UartSendOnePacket(SendCmdPool[SendCmdPoolHead].Buff, SendCmdPool[SendCmdPoolHead].Cnt);
        SendCmdPoolHead++;
        if(SendCmdPoolHead >= CNT_SEND_CMD_POOL)
            SendCmdPoolHead = 0;
    }
}

void UartSendOnePacket(uint8_t *buff, uint32_t cnt)
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

    BuffPtr = SendBuff;
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
    UART_write(UARTHandle, SendBuff, SendCnt);
}


//串口回调，串口数据发送完成后，进行本函数的回调
void Uart_WriteCallback(UART_Handle handle, void *txBuf, size_t size)
{
    //检查是否有数据需要继续发送
    if(SendCmdPoolHead != SendCmdPoolTail)
    {
        //仍然有数据，继续发送
        UartSendOnePacket(SendCmdPool[SendCmdPoolHead].Buff, SendCmdPool[SendCmdPoolHead].Cnt);
        SendCmdPoolHead++;
        if(SendCmdPoolHead >= CNT_SEND_CMD_POOL)
            SendCmdPoolHead = 0;
    }
    else
    {
        //无需继续发送
        UartSendFlag = false;
    }
}

uint32_t ReadActCnt;
void Uart_ReadCallback(UART_Handle handle, void *rxBuf, size_t size)
{
    //int32_t ReadCnt;
    int32_t FirstCnt, SecondCnt;
    
    if(size != 0)
    {
        if(RxBuffTail + size <= SIZE_RX_BUFF)
        {
            memcpy(RxBuff + RxBuffTail, ReadBuff, size);
        }
        else
        {
            FirstCnt = SIZE_RX_BUFF - RxBuffTail;
            SecondCnt = size - FirstCnt;
            memcpy(RxBuff + RxBuffTail, ReadBuff, FirstCnt);
            memcpy(RxBuff, ReadBuff + FirstCnt, SecondCnt);
        }
        RxBuffTail += size;
        RxBuffTail = RxBuffTail % SIZE_RX_BUFF;
    }
    UART_read(UARTHandle, ReadBuff, 100);     
}

//void Uart_ReadCallback(UART_Handle handle, void *rxBuf, size_t size)
//{
//    int32_t ReadCnt;
//    int32_t FirstCnt, SecondCnt;
//    StartTime = Clock_getTicks();
//    ReadCnt = UART_read(UARTHandle, ReadBuff, 100);     
//    StopTime = Clock_getTicks();
//    UsedTime = StopTime - StartTime;
//    if(ReadCnt != 0)
//    {
//        ReadActCnt++;
//        if(RxBuffTail + ReadCnt <= SIZE_RX_BUFF)
//        {
//            memcpy(RxBuff + RxBuffTail, ReadBuff, ReadCnt);
//        }
//        else
//        {
//            FirstCnt = SIZE_RX_BUFF - RxBuffTail;
//            SecondCnt = ReadCnt - FirstCnt;
//            memcpy(RxBuff + RxBuffTail, ReadBuff, FirstCnt);
//            memcpy(RxBuff, ReadBuff + FirstCnt, SecondCnt);
//        }
//        RxBuffTail += ReadCnt;
//        RxBuffTail = RxBuffTail % SIZE_RX_BUFF;
//    }
//}



//对数据进行校验，确认完整后进行通知
void IndicateOneFrame(uint8_t *buff, uint8_t len)
{
    uint8_t i;
    uint8_t crc = 0;
    for(i = 0; i < len; i++)
    {
        crc ^= buff[i];
    }

    if(crc != 0)
        return;

    //SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR3, len - 1, buff);
    memcpy(BleNotifyPool.Node[BleNotifyPool.Tail].Buff, buff, len - 1);
    BleNotifyPool.Node[BleNotifyPool.Tail].Cnt = len - 1;
    BleNotifyPool.Tail++;
    if(BleNotifyPool.Tail >= CNT_NOOD_BLE_NOTIFY_POOL)
        BleNotifyPool.Tail = 0;
}


//对于接收到的串口数据进行处理
//解析后，去除串口传输层协议，
//然后向APP进行通知
#define MAX_BUFF_SIZE_PACKET			21 //20字节内容+1字节的CRC
void UartReceivePacketHandler()
{
    static uint8_t RxPacketBuff[MAX_BUFF_SIZE_PACKET];//最大21字节
    static bool IsGetPacketHead;
    static bool TransferFlag;
    static uint8_t PacketLength;
    uint8_t data;
    while(RxBuffHead != RxBuffTail)
    {
        data = RxBuff[RxBuffHead];
        if(IsGetPacketHead != false)
        {
            if(TransferFlag == false)
            {
                if(data == FRAME_START)
                {
                    //在内容里面发现起始位，则认为前一帧消息没有收到结束位，重新开始内容的记录
                    PacketLength = 0;
                }
                else if(data == FRAME_END)
                {
                    //收到结束符，进行一帧数据的通知
                    IndicateOneFrame(RxPacketBuff, PacketLength);
                    IsGetPacketHead = false;
                }
                else if(data == FRAME_TRANSFER)
                {
                    //收到转义字符，不记录到RxPacketBuff中，PacketLength不增加
                    TransferFlag = true;
                }
                else
                {
                    RxPacketBuff[PacketLength] = data;
                    PacketLength++;
                    if(PacketLength > MAX_BUFF_SIZE_PACKET)
                    {
                        //内容长度超过正常缓冲的长度，必然是同时遗漏了起始位和结束位
                        IsGetPacketHead = false;
                    }
                }
            }
            else
            {
                //上一字节是转义字符
                if(data == FRAME_START || data == FRAME_END || data == FRAME_TRANSFER)
                {
                    RxPacketBuff[PacketLength] = data;
                    PacketLength++;
                    if(PacketLength > MAX_BUFF_SIZE_PACKET)
                    {
                        //内容长度超过正常缓冲的长度，必然是同时遗漏了起始位和结束位
                        IsGetPacketHead = false;
                    }
                }
                else
                {
                    //转移字符后面的字符不是有效字符，此帧错误
                    IsGetPacketHead = false;
                }
                TransferFlag = false;
            }
        }
        else
        {
             if(TransferFlag == false)
             {
                 if(data == FRAME_START)
                 {
                     //找到帧头
                      IsGetPacketHead = true;
                      PacketLength = 0;
                 }
                 else if(data == FRAME_TRANSFER)
                 {
                     //遇到转义，则进入转义判断
                     TransferFlag = true;
                 }
             }
             else
             {
                 TransferFlag = false;
             }
        }
        
        RxBuffHead++;
        if(RxBuffHead >= SIZE_RX_BUFF)
        {
            RxBuffHead = 0;
        }
    }
}


void BleNotifyHandler()
{
    static uint32_t HandleCnt = 0;
    HandleCnt++;
    if(HandleCnt >= 25)
    {
        HandleCnt = 0;
    }
    else
    {
        return;
    }

    if(BleNotifyPool.Head == BleNotifyPool.Tail)
        return;

    
    SimpleProfile_SetParameter(SIMPLEPROFILE_CHAR3, BleNotifyPool.Node[BleNotifyPool.Head].Cnt, BleNotifyPool.Node[BleNotifyPool.Head].Buff);
    BleNotifyPool.Head++;
    if(BleNotifyPool.Head >= CNT_NOOD_BLE_NOTIFY_POOL)
        BleNotifyPool.Head = 0;
}


