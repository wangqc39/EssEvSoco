#include <stdint.h>
#include <string.h>
#include "SystemInfo.h"
#include "SpiFlash.h"
#include "DownLoader.h"
#include "FirmwareConvert.h"
#include "ActionTick.h"




//Downloader占用的FLASH空间大小



//下载器扇区分配
//#define DOWNLOADER_INFO_SECTOR_INDEX			0
//#define DOWNLOADER_CONTENT_SECTOR_INDEX		1


//下载器单个数据包包含数据量
#ifdef PROTOCAL_DD
#define SIZE_ONE_PACKET_CONTENT			14
#define REPORT_CNT_PACKET_LOST				34
#else
#define SIZE_ONE_PACKET_CONTENT			19
#define REPORT_CNT_PACKET_LOST				19
#endif
//一个数据块包含的数据包数量
#define PACKET_CNT_ONE_BLOCK				(BLOCK_SIZE_DATA_TRANSFER / SIZE_ONE_PACKET_CONTENT + 1)
#define DATA_CNT_LAST_PACKET				(BLOCK_SIZE_DATA_TRANSFER % SIZE_ONE_PACKET_CONTENT)

#define MAX_DOWNLOAD_BLOCK_CNT						256 //数据块的最大个数


struct DownloaderInfo
{
    FunctionalState IsEnabled;
    uint8_t BlockBuff[BLOCK_SIZE_DATA_TRANSFER];
    uint8_t PacketReadyBuff[PACKET_CNT_ONE_BLOCK];//块数据传输过程中，对于每个数据包是否接收到的标志列表
    uint32_t TotalDataCnt;
    uint32_t TotalBlockCnt;//总共需要下载的块的数量
    uint32_t InfoAddr;
    uint32_t ContentAddr;
    uint32_t BlockIndexNow;
    uint32_t ThisBlockPacketCnt;//当前数据块包含的数据包个数，在最后一个数据块时会不同
    uint32_t CheckSum;
    uint32_t LastPacketTime;//收到的最后一个下载数据包的时间
};

struct DownloaderInfo DL;

//for test
uint32_t DlStartTime;
uint32_t DlFinishTime;
uint32_t ConverFinishTime;
uint32_t DlTime;
uint32_t ConverTime;

//除法结果向上取整
//data:被除数
//divisor:除数
uint32_t DivCeil(uint32_t data, uint32_t divisor)
{
    return (data + divisor - 1) / divisor;
}


void DownloaderInit()
{
    //初始化下载器的FLASH起始空间
    DL.InfoAddr = SpiFlash.FlashSize - SIZE_DOWNLOADER_AREA;
    DL.ContentAddr = DL.InfoAddr + SPI_FLASH_SECTOR_SIZE;
}

//重置下载器，对下载器的区域进行清空
int32_t RestartDownloader()
{
    uint32_t EraseAddr;
    EraseAddr = DL.InfoAddr;
    while(EraseAddr < DL.InfoAddr + SIZE_DOWNLOADER_AREA)
    {
        BlockErase(EraseAddr);
        EraseAddr += WINBOND_FLASH_BLOCK_SIZE;
    }
    return 0;
}


//DowmloadMode:0-断点续传；1-重新下载
int32_t WriteFirmwareStart(uint32_t FirmwareSize, uint32_t CheckSum, DownloadModeInfo DownloadMode)
{
    uint8_t TmpBuff[LENGTH_DOWNLOADER_FILE_TYPE + LENGTH_DOWNLOADER_FILE_LENGTH + LENGTH_DOWNLOADER_CHECK_SUM];
    int32_t CmpResultFileLength;
    int32_t CmpResultCheckSum;
    uint8_t FileType;
    uint32_t TotalBlockCnt;



    //获取当前下载器中的内容 ,包含下载文件类型和文件信息
    //其中文件信息的长度和本次需要下载的文件信息长度相同
    DataFlashReadData(DL.InfoAddr + OFFSET_DOWNLOADER_FILE_TYPE, TmpBuff, 
                        LENGTH_DOWNLOADER_FILE_TYPE + LENGTH_DOWNLOADER_FILE_LENGTH + LENGTH_DOWNLOADER_CHECK_SUM);
    TotalBlockCnt = DivCeil(FirmwareSize, BLOCK_SIZE_DATA_TRANSFER);
    if(TotalBlockCnt > MAX_DOWNLOAD_BLOCK_CNT)
    {
        return 1;
    }

    DlStartTime = GetSystemTime();

    //比较本次请求的文件和下载器中的信息
    CmpResultFileLength = memcmp(TmpBuff + OFFSET_DOWNLOADER_FILE_LENGTH, &FirmwareSize, LENGTH_DOWNLOADER_FILE_LENGTH);
    CmpResultCheckSum = memcmp(TmpBuff + OFFSET_DOWNLOADER_CHECK_SUM, &CheckSum, LENGTH_DOWNLOADER_CHECK_SUM);
    FileType = TmpBuff[0];

    if(DownloadMode == DOWNLOAD_MODE_REDOWNLOAD || FileType != 1 || CmpResultFileLength != 0 || CmpResultCheckSum != 0)
    {
        //重下载模式
        //或者本次请求下载的文件和下载器中的不同
        //或者本次请求的下载的文件的信息和上次下载的不同
        RestartDownloader();
        TmpBuff[0] = 1;//标识下载文件类型为固件文件
        memcpy(TmpBuff + OFFSET_DOWNLOADER_FILE_LENGTH, &FirmwareSize, LENGTH_DOWNLOADER_FILE_LENGTH);
        memcpy(TmpBuff + OFFSET_DOWNLOADER_CHECK_SUM, &CheckSum, LENGTH_DOWNLOADER_CHECK_SUM);
        DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOADER_FILE_TYPE, TmpBuff, 
                              LENGTH_DOWNLOADER_FILE_TYPE + LENGTH_DOWNLOADER_FILE_LENGTH + LENGTH_DOWNLOADER_CHECK_SUM);
    }
    else
    {
        //所有信息匹配上，并且断点续传
        //开始断点续传
        //上次由于某种原因下载中断
        //或者重复进行下载
        //所以不需要对DOWNLOADER_INFO_SECTOR做任何改动。
    }

    DL.TotalDataCnt = FirmwareSize;
    DL.TotalBlockCnt = TotalBlockCnt;
    DL.BlockIndexNow = 0;
    
    DL.IsEnabled = ENABLE;

    DL.CheckSum = 0;

    DL.LastPacketTime = GetSystemTime();
    
    return 0;
}

//获取下一个传输数据块的索引
int32_t GetNextFirmwareBlockIndex()
{
    int32_t NextBlockIndex = 10000;
    uint8_t BlockUsage[MAX_DOWNLOAD_BLOCK_CNT];
    int32_t i;

    if(DL.IsEnabled == DISABLE)
        return -1;

    //获取下载器中IndexReady的表，其中每1字节表示一块数据是否已写入，0表示已经写入
    DataFlashReadData(DL.InfoAddr + OFFSET_DOWNLOADER_INDEX_READY_FLAG, 
                                     BlockUsage, DL.TotalBlockCnt);

    //找到第一个没有写入的Block
    for(i = 0; i < DL.TotalBlockCnt; i++)
    {
        if(BlockUsage[i] != 0)
        {
            NextBlockIndex = i;
            break;
        }
    }

    if(NextBlockIndex != 10000)
    {
        //有空闲没有接收的BLOCK
        DL.BlockIndexNow = NextBlockIndex;
        //清空接收到数据包的列表，准备开始接收数据包
        memset(DL.PacketReadyBuff, 0, PACKET_CNT_ONE_BLOCK);
        if(NextBlockIndex == DL.TotalBlockCnt - 1)
        {
            //当前数据块是最后一个数据块
            if(DL.TotalDataCnt % BLOCK_SIZE_DATA_TRANSFER == 0)
            {
                DL.ThisBlockPacketCnt = DivCeil(BLOCK_SIZE_DATA_TRANSFER, SIZE_ONE_PACKET_CONTENT);
            }
            else
            {
                DL.ThisBlockPacketCnt = DivCeil(DL.TotalDataCnt % BLOCK_SIZE_DATA_TRANSFER, SIZE_ONE_PACKET_CONTENT);
            }
            
        }
        else
        {
            //当前数据块不是最后一个数据块
            DL.ThisBlockPacketCnt = DivCeil(BLOCK_SIZE_DATA_TRANSFER, SIZE_ONE_PACKET_CONTENT);
        }
    }


    DL.LastPacketTime = GetSystemTime();
    
    return NextBlockIndex;
}


int32_t WriteFirmwareBlockData(uint8_t *buff, uint32_t length)
{
    uint8_t Index;
    int32_t LastLength;
    if(DL.IsEnabled == DISABLE)
        return -1;

    Index = buff[0];
    //确认数据包长度合法
    if(Index + 1 < DL.ThisBlockPacketCnt)
    {
        //当前数据包不是块的最后一个数据包，数据包长度固定
        if(length - 1 != SIZE_ONE_PACKET_CONTENT)
            return -2;
    }
    else if(Index + 1 == DL.ThisBlockPacketCnt)
    {
        //当前是本块的最后一个数据包
        if(DL.BlockIndexNow + 1 < DL.TotalBlockCnt)
        {
            //当前块不是最后一块,数据长度固定
            if(length - 1 != DATA_CNT_LAST_PACKET)
                return -2;
        }
        else
        {
            //当前块是最后一个块的最后一个数据包，长度需要计算
            LastLength = (DL.TotalDataCnt % BLOCK_SIZE_DATA_TRANSFER) % SIZE_ONE_PACKET_CONTENT;
            if(LastLength == 0)
                LastLength = DATA_CNT_LAST_PACKET;
            if(length - 1 != LastLength)
                return -3;
        }
        
    }
    else
    { 
        //index溢出
        return -4;
    }




    //将数据拷贝到本数据块的缓冲中
    memcpy(DL.BlockBuff + Index * SIZE_ONE_PACKET_CONTENT, buff + 1, length -1);
    DL.PacketReadyBuff[Index] = 1;

    DL.LastPacketTime = GetSystemTime();
    
    return 0;
}

int32_t CheckFirmwareBlockData(uint8_t BlockIndex, uint8_t *LostPacketIndex)
{
    int32_t LostPacketCnt;
    int32_t i;
    const uint8_t ZeroData = 0;

    //检查合法性
    if(DL.IsEnabled == DISABLE)
        return -1;

    if(BlockIndex != DL.BlockIndexNow)
        return -2;

    LostPacketCnt = 0;
    for(i = 0; i < DL.ThisBlockPacketCnt; i++)
    {
        if(DL.PacketReadyBuff[i] == 0)
        {
            //记录丢失的数据包
            LostPacketIndex[LostPacketCnt] = i;
            LostPacketCnt++;
            //一次最多上报19个丢失的数据包
            if(LostPacketCnt >= REPORT_CNT_PACKET_LOST)
                break;
        }
    }

    if(LostPacketCnt == 0)
    {
        uint32_t WriteCnt;
        //数据包全部接收到，对数据进行写入
        if(BlockIndex + 1 != DL.TotalBlockCnt)
        {
            //当前块不是最后一个数据块，全缓冲写入
            WriteCnt = BLOCK_SIZE_DATA_TRANSFER;
        }
        else
        {
            //当前块是最后一个数据块，写入数量需要计算
            WriteCnt = DL.TotalDataCnt % BLOCK_SIZE_DATA_TRANSFER;
            if(WriteCnt == 0)
                WriteCnt = BLOCK_SIZE_DATA_TRANSFER;
        }
        DataFlashWriteData(DL.ContentAddr + DL.BlockIndexNow * BLOCK_SIZE_DATA_TRANSFER, 
                                          DL.BlockBuff, WriteCnt);
        
        //对当前Block的Flag进行写入
        DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOADER_INDEX_READY_FLAG + DL.BlockIndexNow,
                                          (uint8_t *)&ZeroData, 1);

        uint32_t CheckCnt = WriteCnt /4;
        uint32_t CheckData;
        for(i = 0; i < CheckCnt; i++)
        {
            memcpy(&CheckData, DL.BlockBuff + i * 4, 4);
            DL.CheckSum ^= CheckData;
        }
        
    }


    DL.LastPacketTime = GetSystemTime();
        
    return LostPacketCnt;
}

int32_t WriteFirmwareFinish()
{
    int32_t NextBlockIndex;
    //uint8_t DownFinishFlag;
    //uint8_t ConvertFinishFlag;
    //const uint8_t ZeroData = 0;
    int32_t ret = 0;

    NextBlockIndex = GetNextFirmwareBlockIndex();
    if(NextBlockIndex < 0)
    {
        //错误
        ret = NextBlockIndex;
        goto end;
    }
    else if(NextBlockIndex != 10000)
    {
        //尚有数据块没有写入
        ret = -2;
        goto end;
    }
    else
    {
        //数据已全部写入
    }


    DlFinishTime = GetSystemTime();
    DlTime = (DlFinishTime - DlStartTime) / 10;


    //Todo
    ret = FirmwareFileConvert(DL.InfoAddr, DL.TotalDataCnt);
    if(ret != 0)
    {
        goto end;
    }

    ConverFinishTime = GetSystemTime();
    ConverTime = (ConverFinishTime - DlFinishTime) / 10;

    DL.LastPacketTime = GetSystemTime();
end:
    
    DL.IsEnabled = DISABLE;
    return ret;
}



FunctionalState IsFirmwareDownloading()
{
    return DL.IsEnabled;
}

uint32_t GetLastDownloadMessageTime()
{
    return DL.LastPacketTime;
}

void StopDonwload()
{
    DL.IsEnabled = DISABLE;
}




