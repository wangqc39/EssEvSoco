#include <stdint.h>
#include <string.h>
#ifdef PROTOCAL_DD
#include "BleDDTransportLayer.h"
#else
#include "BleTransportLayer.h"
#endif

#define ERROR_FLAG										0xFF

void BleSendErrorPacket(uint8_t MessageId, uint32_t ErrorCode)
{
    uint8_t ErrorPacketBuff[5];
    ErrorPacketBuff[0] = MessageId;
    ErrorPacketBuff[1] = ERROR_FLAG;
    memcpy(ErrorPacketBuff + 2, &ErrorCode, sizeof(uint32_t));
    BleSendOneFrame(ErrorPacketBuff, 6);
}
