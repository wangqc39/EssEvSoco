#ifndef __SPI_FLASH__
#define __SPI_FLASH__

#include "stm32f10x_lib.h"
#include <stdint.h>

/*#define MAX_FLASH_BLOCK_CNT				256

#define SPI_FLASH_PAGE_SIZE					256
#define SPI_FLASH_BLOCK_SIZE			(64 * 1024)
#define SPI_FLASH_SECTOR_SIZE			(4096)*/
#define SPI_FLASH_PAGE_SIZE					256
#define SPI_FLASH_SECTOR_SIZE			(4096)

//#define FS_SECTOR_SIZE					512

#define WINBOND_FLASH_BLOCK_SIZE			(64 * 1024)
#define WINBOND_FLASH_SECTOR_SIZE			(4096)

//地址计算
#define BlockAddr(x)						(x * 64 * 1024)
#define SectorAddr(x)						(x * 4096)
 
//BLOCK0 - index0
//Sector0 系统配置扇区
#define SECTOR_SYSTEM_CONFIG			0
//Sector1-2 声音配置
#define SECTOR_SOUND_CONFIG(x)			(SECTOR_SYSTEM_CONFIG + 1 + x)
//Sector5 FS文件索引表
#define SECTOR_FAT_TABLE				8
//Sector6-15 文件索引
#define SECTOR_FILE(x)					(SECTOR_FAT_TABLE + 1 + x)


struct SpiFlashInfo
{
    unsigned int Ssid;
    u32 FlashBlockCnt;
    u32 FlashBlockSize;
    u32 FlashSize;
    FunctionalState FlashStatus;
};

extern struct SpiFlashInfo SpiFlash;

void SpiFlashHwInit(void);
uint32_t DataFlashReadData(uint32_t StartAddress, uint8_t *ucRdDataBuff, uint32_t ReadCnt);
int32_t DataFlashWriteData(uint32_t Addr, uint8_t *ucWrDataBuff, uint32_t DataCnt);
void SectorErase(uint32_t Addr);
void BlockErase(uint32_t Addr);
void DataFlashDirectWriteData(uint32_t StartAddress, uint8_t *ucWrDataBuff, uint32_t WriteCnt);

int32_t CheckEmpty(uint8_t *buff, int32_t cnt);

#endif
