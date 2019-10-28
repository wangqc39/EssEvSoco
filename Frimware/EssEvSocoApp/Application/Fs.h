#ifndef __FS__
#define __FS__

#include "common.h"
#include "SpiFlash.h"
#include "SystemInfo.h"

typedef enum {TYPE_ENGINE_SOUND = 0, TYPE_BRAKE_SOUND = 1, TYPE_TURBO_SOUND = 2, TYPE_AUX_SOUND = 3} SoundTypeInfo;

#define FS_SUPPORT_FILE					VEHICLE_CNT
#define ENGINE_INDEX(x)					(x)
//#define BRAKE_INDEX(x)					(6)
//#define TURBO_INDEX(x)					(6)
//#define AUX_INDEX(x)						(6)

#define FS_SUPPORT_MAX_SIZE				(8 * 1024 * 1024)
#define FS_BLOCK_SIZE					(WINBOND_FLASH_BLOCK_SIZE)
#define FS_SUPPORT_MAX_BLOCK_CNT		(FS_SUPPORT_MAX_SIZE / FS_BLOCK_SIZE)// 128

#define FS_ONE_FILE_MAX_SIZE			(4 * 1024 * 1024)


//单个文件索引
#define OFFSET_FILE_ENABLE_FLAG			0
#define OFFSET_FILE_NAME					1
#define OFFSET_FILE_LENGTH				50
#define OFFSET_BLOCK_CNT				54
#define OFFSET_FILE_GUID					58
#define OFFSET_FILE_WHOLE_FLAG			74
#define OFFSET_BLOCK_INDEX(x)			(128 + 2 * x)
#define MAX_BLOCK_INDEX_CNT					192
//单个文件item长度
#define LENGTH_FILE_LENGTH				4
#define LENGTH_BLOCK_CNT				4
#define SIZE_GUID						16
#define SIZE_SOUND_NAME					40
#define SIZE_ENABLE_FLAG					1


struct FileInfo
{
    FunctionalState EnableFlag;
    unsigned char FileIndex;
    unsigned short FileIndexAddr;
    unsigned int FileLength;//在文件写入过程中，表示写入的Index
    unsigned int TotalBlockCnt; //文件总共的索引块数量，在写入时，表示已经分配的数据块
    unsigned short ReadingIndex;//正在读取的block索引号
    unsigned short ReadingBlock;//正在读取的block绝对号码
    unsigned short WritintIndex;
    unsigned short WritingBlock;
    unsigned short BlockTable[FS_ONE_FILE_MAX_SIZE / FS_BLOCK_SIZE];//64 * 64 = 4M,支持单个最大文件4M
    uint32_t FreeDataCntInBlock;//当前块中，尚空余可以写入的字节数
    uint32_t ReadSeek;
};


extern struct FileInfo File[FS_SUPPORT_FILE];

void InitFs(void);
int ReadFile(struct FileInfo *Fp, unsigned int offset, unsigned char *buff, unsigned int ReadCnt);
struct FileInfo *OpenFile(unsigned char FileIndex);
void RecoverBadAudio(void);
int WriteFile(struct FileInfo *Fp, unsigned int offset, unsigned char *buff, unsigned int WriteCnt);
int CreateFile(struct FileInfo *Fp, unsigned int FileLength);
unsigned int GetFsSize(void);
unsigned int GetFsEmptySize(void);
int DeleteFile(struct FileInfo *Fp);
//void ReadSoundConfig(unsigned char SoundIndex, unsigned int offset, unsigned char *buff, unsigned int length);
//int WriteSoundConfig(unsigned char SoundIndex, unsigned int offset, unsigned char *buff, unsigned int length);
//void ReadSystemConfig(unsigned int offset, unsigned char *buff, unsigned int length);
//int WriteSystemConfig(unsigned int offset, unsigned char *buff, unsigned int length);

unsigned int  GetFileLength(struct FileInfo *Fp);
u16 FormatFs(void);
void GetFsInfo(void);


int32_t WriteParamSector(uint8_t *ParamTable, uint32_t ParamAddr);


int32_t CreateEmptyFile(struct FileInfo *Fp);
int WriteFileWithAlloc(struct FileInfo *Fp, unsigned char *buff, unsigned int WriteCnt);
void InitFile(struct FileInfo *Fp);


#endif


