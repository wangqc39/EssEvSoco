#ifndef __UART_H__
#define __UART_H__

void UartSendPacket(uint8_t *buff, uint32_t cnt);
void UartHwInit();
void UartReceivePacket();
void UartReceivePacketHandler();
void BleNotifyHandler();

#endif


