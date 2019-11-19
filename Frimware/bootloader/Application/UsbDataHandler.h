#ifndef __USB_DATA_HANDLER__
#define __USB_DATA_HANDLER__

typedef enum {START_BIT1 = 0, START_BIT2 = 1, LENGTH_HIGH = 2, LENGTH_LOW = 3, 
                         CONTENT = 3, CHECK_BIT = 4, END_BIT = 5} TransParseFlag;

typedef enum {USB_TRANS = 0, COM_TRANS = 1} TransTypeFlag;
typedef enum {NO_BATCH = 0, BATCH_WRITE = 1, BATCH_READ = 2} BatchTypeFlag;

#define MAX_ENGINE_NUM	4
#define ONE_ENGINE_INFO_LENGTH	24
//引擎声音信息前有1字节的标识引擎数量
#define ENGINE_INFO_OFFSET			1
#define ENGINE_NAME_OFFSET		4

#define MAX_SPECIAL_SOUND_CNT				2
#define SPECIAL_SOUND_INFO_LENGTH		128






void UsbDataAnalyze(unsigned char *buff);
void UsbTransLayerHandler(void);
void UsbSerialDataLayerHandler(unsigned char *buff, unsigned short int cnt);


//数据层宏定义
#define ERROR_CMD					0x29
#define WRITE_SOUND_DATA_CMD		0x30
#define FLASH_INFO_READ_CMD		0x31
#define WRITE_SOUND_DATA_FINISH_CMD		0x32
#define WRITE_SOUND_DATA_START_CMD		0x33
#define WRITE_APPLICATION_DATA_CMD	0x38
#define WRITE_APPLICATION_DATA_FINISH_CMD		0x39
#define WRITE_APPLICATION_DATA_START_CMD		0x3A
#define RUN_TO_APPLICATION			0x3B
#define READ_ENGINE_FLASH_SPACE_CMD		0x3C
#define READ_ENGINE_NAME_CMD		0x3D
#define BOOTLOADER_SELF_UPDATE_CMD		0x3E
#define WRITE_SELF_UPDATE_DATA_CMD			0x3F
#define WRITE_SELF_UPDATE_DATA_FINISH_CMD		0x40
#define WRITE_SELF_UPDATE_DATA_START_CMD		0x41
#define READ_SYSTEM_CFG_CMD		0x46
#define WRITE_SYSTEM_CFG_CMD		0x47
#define READ_AUDIO_CFG_CMD		0x48
#define WRITE_AUDIO_CFG_CMD		0x49
#define READ_SERIAL_NUM_CMD		0x4A
#define CHECK_WORKING_PROGRAMME_CMD	0x4B
//#define GET_HARDWARE_VERSION		0x4E
#define STORAGE_WRITE_STATUS_CMD		0x70
#define STORAGE_READ_STATUS_CMD		0x71
#define READ_SYSTEM_INFO				0x75
#define BATCH_WRITE_CMD				0x76
#define BATCH_READ_CMD					0x77
#define HW_SET_CMD						0x78
#endif




