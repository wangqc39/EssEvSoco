#ifndef __BLE_ERROR__
#define __BLE_ERROR__

#define ERROR_SYSTEM_INFO_REQUEST_LENGTH				0x01010000
#define ERROR_SYSTEM_INFO_REQUEST_INDEX				0x01020000
#define ERROR_SYSTEM_PARAM_REQUEST_LENGTH			0x01030000
#define ERROR_SYSTEM_PARAM_REQUEST_INDEX				0x01040000
#define ERROR_VEHICLE_PARAM_REQUEST_LENGTH			0x01050000
#define ERROR_VEHICLE_PARAM_REQUEST_INDEX				0x01060000
#define ERROR_SYSTEM_PARAM_SET_LENGTH					0x01070000
#define ERROR_SYSTEM_PARAM_SET_INDEX					0x01080000
#define ERROR_VEHICLE_PARAM_SET_LENGTH					0x01070000
#define ERROR_VEHICLE_PARAM_SET_INDEX					0x01080000
#define ERROR_VEHICLE_PARAM_REQUEST_VEHICLE_INDEX		0x01090000
#define ERROR_VEHICLE_PARAM_SET_VEHICLE_INDEX			0x010A0000
#define ERROR_SYSTEM_PARAM_STORE_LENGTH				0x010B0000
#define ERROR_VEHICLE_PARAM_STORE_LENGTH				0x010C0000
#define ERROR_VEHICLE_PARAM_STORE_VEHICLE_INDEX		0x010D0000
#define ERROR_FORMAT_FS									0x010E0000
#define ERROR_READ_SOUND_GUID_INDEX					0x010F0000
#define ERROR_READ_SOUND_GUID_SOUND_TYPE				0x02000000
#define ERROR_READ_SOUND_NAME_INDEX					0x02010000
#define ERROR_READ_SOUND_NAME_SOUND_TYPE			0x02020000
#define ERROR_WRITE_SOUND_START_TOTAL_PACKET_CNT		0x02030000
#define ERROR_WRITE_SOUND_START_FIRST_PACKET_INDEX	0x02040000
#define ERROR_WRITE_SOUND_START_CONTINUE_PACKET_INDEX	0x02050000
#define ERROR_WRITE_SOUND_START_TOTAL_LENGTH			0x02060000
#define ERROR_WRITE_SOUND_START_TOTAL_PACKET_CNT_CONTINUE		0x02070000
#define ERROR_WRITE_SOUND_START_TOTAL_LENGTH_CONTINUE	0x02080000
#define ERROR_WRITE_SOUND_START_WRITE_SOUND_START	0x02090000
#define ERROR_GET_NEXT_SOUND_BLOCK_INDEX				0x020A0000
#define ERROR_CHECK_SOUND_BLOCK_DATA_LENGTH			0x020B0000
#define ERROR_CHECK_SOUND_BLOCK_DATA					0x020C0000
#define ERROR_WRITE_SOUND_FINISH						0x020D0000
#define ERROR_DELETE_SOUND_LENGTH						0x020E0000
#define ERROR_DELETE_SOUND_VEHICLE_INDEX				0x020F0000
#define ERROR_DELETE_SOUND_SOUND_TYPE				0x03000000
#define ERROR_DELETE_SOUND_SOUND						0x03010000
#define ERROR_SELECT_SOUND_LENGTH						0x03020000
#define ERROR_SELECT_SOUND_INDEX						0x03030000


#define ERROR_WRITE_FIRMWARE_START_LENGTH               0x10010000
#define ERROR_WRITE_FIRMWARE_START_FIRMWARE_SIZE        0x10020000
#define ERROR_WRITE_FIRMWARE_START                      0x10030000
#define ERROR_GET_NEXT_FIRMWARE_BLOCK_INDEX				0x10040000
#define ERROR_CHECK_FIRMWARE_BLOCK_DATA_LENGTH			0x10050000
#define ERROR_CHECK_FIRMWARE_BLOCK_DATA					0x10060000
#define ERROR_WRITE_FIRMWARE_FINISH						0x10070000
#define ERROR_WRITE_FIRMWARE_START_FIRMWARE_MODE        0x10080000
#define ERROR_WRITE_FIRMWARE_START_FIRMWARE_SIZE_ALIGN  0x10090000




void BleSendErrorPacket(uint8_t MessageId, uint32_t ErrorCode);

#endif
