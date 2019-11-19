#include "main.h"
#include <stdint.h>
#include "BleComHw.h"
#include "BleTransportLayer.h"
#include "BleDataLayer.h"

/*COM物理层处理*/
#define BLE_COM_PORT		USART3
struct BleComInfo BleCom;


void BleComEnable(FunctionalState flag)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = flag;
    NVIC_Init(&NVIC_InitStructure);
    USART_ITConfig(BLE_COM_PORT, USART_IT_RXNE, flag);
    USART_Cmd(BLE_COM_PORT, flag);
}

void BleComHwInit()
{
    USART_InitTypeDef USART_InitStructure; 
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_PinRemapConfig(GPIO_PartialRemap_USART3, ENABLE   );
    
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStructure.USART_BaudRate = 115200;

    USART_Init(BLE_COM_PORT, &USART_InitStructure);
    //USART_HalfDuplexCmd(BLE_COM_PORT, ENABLE);
    BleComEnable(ENABLE);
}

void BleComIntHandler()
{
    //if(BLE_COM_PORT->SR & 0x0020)
    if(USART_GetITStatus(BLE_COM_PORT, USART_IT_RXNE))
    {
        BleCom.RxBuff[BleCom.RxTail] = BLE_COM_PORT->DR;//USART_ReceiveData(UART4);
        BleCom.RxTail++;
        if(BleCom.RxTail >= BLE_COM_RX_SIZE)
        {
            BleCom.RxTail = 0;
        }
        USART_ClearITPendingBit(BLE_COM_PORT, USART_IT_RXNE);
        //BLE_COM_PORT->SR = ~0x0020;
    }
    //else if(USART_GetITStatus(UART4, USART_IT_TC))
    //if(BLE_COM_PORT->SR & 0x0040)
    else if(USART_GetITStatus(BLE_COM_PORT, USART_IT_TXE))
    {
        if(BleCom.TxHead == BleCom.TxTail)
        {
            //数据发送完成，关闭数据发送完成中断
            //BLE_COM_PORT->CR1 &= ~USART_ISR_TXE;
            USART_ITConfig(BLE_COM_PORT, USART_IT_TXE, DISABLE);
            BleCom.IsTxing = DISABLE;
        }
        else
        {
            BLE_COM_PORT->DR = BleCom.TxBuff[BleCom.TxHead];
            BleCom.TxHead++;
            if(BleCom.TxHead >= BLE_COM_TX_SIZE)
            {
                BleCom.TxHead = 0;
            }
        }
    }
}


void BleComSendData(unsigned char *buff, unsigned short int cnt)
{
    int i;
    for(i = 0; i < cnt; i++)
    {
        BleCom.TxBuff[BleCom.TxTail] = buff[i];
        BleCom.TxTail++;
        if(BleCom.TxTail >= BLE_COM_TX_SIZE)
        {
            BleCom.TxTail = 0;
        }
    }

    if(BleCom.IsTxing == DISABLE)
    {
        BleCom.IsTxing = ENABLE;

        USART_SendData(BLE_COM_PORT, BleCom.TxBuff[BleCom.TxHead]);
        BleCom.TxHead++;
        if(BleCom.TxHead >= BLE_COM_TX_SIZE)
        {
            BleCom.TxHead = 0;
        }
        USART_ITConfig(BLE_COM_PORT, USART_IT_TXE, ENABLE);
    }
}


