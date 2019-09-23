#ifndef __SPI_FLASH__
#define __SPI_FLASH__

#define MAX_FLASH_BLOCK_CNT				256

#define SPI_FLASH_PAGE_SIZE					256
#define SPI_FLASH_BLOCK_SIZE			(64 * 1024)
#define SPI_FLASH_SECTOR_SIZE			(4096)


#define WINBOND_FLASH_BLOCK_SIZE			SPI_FLASH_BLOCK_SIZE
#define WINBOND_FLASH_SECTOR_SIZE			SPI_FLASH_SECTOR_SIZE



//地址计算
#define BlockAddr(x)						((x) * SPI_FLASH_BLOCK_SIZE)
#define SectorAddr(x)						((x) * SPI_FLASH_SECTOR_SIZE)

 
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
    uint32_t Ssid;
    uint32_t FlashBlockCnt;
    uint32_t FlashBlockSize;
    uint32_t FlashSize;
    FunctionalState FlashStatus;
};

extern struct SpiFlashInfo SpiFlash;

void SpiFlashHwInit(void);
uint32_t DataFlashReadData(uint32_t StartAddress, uint8_t *ucRdDataBuff, uint32_t ReadCnt);
uint32_t DataFlashWriteData(uint32_t Addr, uint8_t *ucWrDataBuff, uint32_t DataCnt);
void SectorErase(uint32_t Addr);
void BlockErase(uint32_t Addr);
void DataFlashDirectWriteData(uint32_t StartAddress, uint8_t *ucWrDataBuff, uint32_t WriteCnt);

int32_t CheckEmpty(uint8_t *buff, int32_t cnt);

#endif

