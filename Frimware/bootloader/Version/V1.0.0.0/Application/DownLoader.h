#ifndef __DOWNLOADER__
#define __DOWNLOADER__
#include "ActionTick.h"

/*
#define LENGTH_FILE_LENGTH                          4
#define LENGTH_CHECK_SUM                            4
#define LENGTH_DOWNLOAD_MODE                        1
*/

//下载器信息扇区空间分配
#define OFFSET_DOWNLOADER_FILE_TYPE					0
#define LENGTH_DOWNLOADER_FILE_TYPE					1

#define OFFSET_DOWNLOADER_FILE_LENGTH				(OFFSET_DOWNLOADER_FILE_TYPE + LENGTH_DOWNLOADER_FILE_TYPE)
#define LENGTH_DOWNLOADER_FILE_LENGTH				4//LENGTH_FILE_LENGTH//  4

#define OFFSET_DOWNLOADER_CHECK_SUM				    (OFFSET_DOWNLOADER_FILE_LENGTH + LENGTH_DOWNLOADER_FILE_LENGTH)
#define LENGTH_DOWNLOADER_CHECK_SUM				    4//LENGTH_CHECK_SUM




#define OFFSET_DOWNLOADER_INDEX_READY_FLAG			512




#define SIZE_DOWNLOADER_AREA				(512 * 1024)


#define DOWNLOAD_TIME_OUT_TIME			(1000 * TIMER_MULTI)


typedef enum {DOWNLOAD_MODE_BREAK_POINT = 0, DOWNLOAD_MODE_REDOWNLOAD = 1} DownloadModeInfo;

int32_t WriteFirmwareStart(uint32_t FirmwareSize, uint32_t CheckSum, DownloadModeInfo DownloadMode);

void DownloaderInit(void);
int32_t GetNextFirmwareBlockIndex(void);
int32_t WriteFirmwareBlockData(uint8_t *buff, uint32_t length);
int32_t CheckFirmwareBlockData(uint8_t BlockIndex, uint8_t *LostPacketIndex);
int32_t WriteFirmwareFinish(void);
FunctionalState IsAudioDownloading(void);
uint32_t GetLastDownloadMessageTime(void);
void StopDonwload(void);
uint32_t DivCeil(uint32_t data, uint32_t divisor);


#endif

