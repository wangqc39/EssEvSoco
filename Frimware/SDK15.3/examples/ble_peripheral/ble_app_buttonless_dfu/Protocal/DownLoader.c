#include <stdint.h>
#include <string.h>
#include "SystemInfo.h"
#include "SpiFlash.h"
#include "DownLoader.h"
#include "AudioFileConvert.h"
#include "ActionTick.h"




//Downloader占用的FLASH空间大小



//下载器扇区分配
//#define DOWNLOADER_INFO_SECTOR_INDEX			0
//#define DOWNLOADER_CONTENT_SECTOR_INDEX		1


//下载器单个数据包包含数据量
#define SIZE_ONE_PACKET_CONTENT_BLE4			19
#define SIZE_ONE_PACKET_CONTENT_BLE42           (244 - 1)
#define REPORT_CNT_PACKET_LOST				19

//一个数据块包含的数据包数量
//#define PACKET_CNT_ONE_BLOCK				(BLOCK_SIZE_DATA_TRANSFER / SIZE_ONE_PACKET_CONTENT + 1)
//#define DATA_CNT_LAST_PACKET				(BLOCK_SIZE_DATA_TRANSFER % SIZE_ONE_PACKET_CONTENT)

//DL区域总共512K，DL.Info区域4K大小
#define MAX_DOWNLOAD_BLOCK_CNT			   256//((SIZE_DOWNLOADER_AREA - SPI_FLASH_SECTOR_SIZE) / 1024) //数据块的最大个数


struct DownloaderInfo
{
    FunctionalState IsEnabled;
    uint8_t BlockBuff[BLOCK_SIZE_DATA_TRANSFER];
    uint8_t PacketReadyBuff[(BLOCK_SIZE_DATA_TRANSFER / SIZE_ONE_PACKET_CONTENT_BLE4 + 1)];//块数据传输过程中，对于每个数据包是否接收到的标志列表
    uint32_t TotalDataCnt;
    uint32_t TotalBlockCnt;//总共需要下载的块的数量
    uint32_t InfoAddr;
    uint32_t ContentAddr;
    uint32_t BlockIndexNow;
    uint32_t ThisBlockPacketCnt;//当前数据块包含的数据包个数，在最后一个数据块时会不同
    uint32_t ThisBlockDataCnt;//当前数据块包含的数据个数
    //uint32_t CheckSum;
    uint32_t LastPacketTime;//收到的最后一个下载数据包的时间
    uint32_t PacketDataCnt;//用于记录一个数据包的数据数量，有19字节和243字节两种
};

struct DownloaderInfo DL = 
{
    .PacketDataCnt = 19, //默认为19，支持BLE4.0
};

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



int32_t WriteSoundStart(uint8_t *buff, uint32_t length)
{
    uint8_t TmpBuff[LENGTH_DOWNLOADER_FILE_LENGTH + LENGTH_DOWNLOADER_VEHICLE_INDEX + 
                              LENGTH_DOWNLOADER_SOUND_TYPE + LENGHT_DOWNLOADER_GUID + LENGTH_DOWNLOADER_SOUND_NAME_MAX + 1];
    int32_t CmpResultFileLength;
    int32_t CmpResultVehicleIndex;
    int32_t CmpResultOtherInfo;
    uint8_t FileType;
    uint32_t TotalDataCnt;
    uint32_t TotalBlockCnt;
    uint8_t VehicleIndex;
    uint8_t SoundType;
    const uint8_t ClearData = 0xFF;


    //获取当前下载器中的内容 ,包含下载文件类型和文件信息
    //其中文件信息的长度和本次需要下载的文件信息长度相同
    DataFlashReadData(DL.InfoAddr + OFFSET_DOWNLOADER_FILE_TYPE, TmpBuff, length + 1);
    memcpy(&TotalDataCnt, buff + OFFSET_DOWNLOADER_FILE_LENGTH - 1, LENGTH_DOWNLOADER_FILE_LENGTH);
    memcpy(&VehicleIndex, buff + OFFSET_DOWNLOADER_VEHICLE_INDEX - 1, LENGTH_DOWNLOADER_VEHICLE_INDEX);
    memcpy(&SoundType, buff + OFFSET_DOWNLOADER_SOUND_TYPE - 1, LENGTH_DOWNLOADER_SOUND_TYPE);
    TotalBlockCnt = DivCeil(TotalDataCnt, BLOCK_SIZE_DATA_TRANSFER);
    if(TotalBlockCnt > MAX_DOWNLOAD_BLOCK_CNT)
    {
        return 1;
    }
    if(VehicleIndex > VEHICLE_CNT)
    {
        return 2;
    }
    if(SoundType > TYPE_ENGINE_SOUND)
    {
        return 3;
    }

    DlStartTime = GetSystemTime();

    //比较本次请求的文件和下载器中的信息
    CmpResultFileLength = memcmp(TmpBuff + OFFSET_DOWNLOADER_FILE_LENGTH, buff + OFFSET_DOWNLOADER_FILE_LENGTH - 1, LENGTH_DOWNLOADER_FILE_LENGTH);
    CmpResultVehicleIndex = memcmp(TmpBuff + OFFSET_DOWNLOADER_VEHICLE_INDEX, buff + OFFSET_DOWNLOADER_VEHICLE_INDEX - 1, LENGTH_DOWNLOADER_VEHICLE_INDEX);
    CmpResultOtherInfo = memcmp(TmpBuff + OFFSET_DOWNLOADER_SOUND_TYPE, buff + OFFSET_DOWNLOADER_SOUND_TYPE - 1, length - LENGTH_DOWNLOADER_FILE_LENGTH - LENGTH_DOWNLOADER_VEHICLE_INDEX);
    FileType = TmpBuff[0];
    
    if(FileType != 0 || CmpResultFileLength != 0 || CmpResultOtherInfo != 0)
    {
        //本次请求下载的信息和下载器中的不同
        //或者下载器中下载的不是声音文件
        //则对下载器进行重置
        RestartDownloader();
        TmpBuff[0] = 0;//标识下载文件类型为声音文件
        memcpy(TmpBuff + 1, buff, length);
        DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOADER_FILE_TYPE, TmpBuff, length + 1);
    }
    else if(CmpResultVehicleIndex != 0)
    {
        uint8_t VehicleIndex;
        VehicleIndex = buff[OFFSET_DOWNLOADER_VEHICLE_INDEX - 1];
        //声音文件的信息完全相同
        //但是要下载的声音的VehicleIndex不同
        //仍然能够继续下载，仅在此修改下载器中的VehicleIndex
        DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOADER_VEHICLE_INDEX, 
                                         &VehicleIndex, LENGTH_DOWNLOADER_VEHICLE_INDEX);

        //清除转换完成的标志位
        DataFlashWriteData(DL.InfoAddr + OFFSET_AUDIO_CONVERT_FLAG,
                                          (uint8_t *)&ClearData, LENGTH_AUDIO_CONVERT_FLAG);
    }
    else
    {
        //所有信息均匹配上
        //上次由于某种原因下载中断
        //或者重复进行下载
        //所以不需要对DOWNLOADER_INFO_SECTOR做任何改动。
    }

    DL.TotalDataCnt = TotalDataCnt;
    DL.TotalBlockCnt = TotalBlockCnt;
    DL.BlockIndexNow = 0;
    
    DL.IsEnabled = ENABLE;

    //DL.CheckSum = 0;

    DL.LastPacketTime = GetSystemTime();
    
    return 0;
}

//获取下一个传输数据块的索引
int32_t GetNextSoundBlockIndex()
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
        memset(DL.PacketReadyBuff, 0, (BLOCK_SIZE_DATA_TRANSFER / SIZE_ONE_PACKET_CONTENT_BLE4 + 1));
        if(NextBlockIndex == DL.TotalBlockCnt - 1)
        {
            //当前数据块是最后一个数据块
            if(DL.TotalDataCnt % BLOCK_SIZE_DATA_TRANSFER == 0)
            {
                //DL.ThisBlockPacketCnt = DivCeil(BLOCK_SIZE_DATA_TRANSFER, SIZE_ONE_PACKET_CONTENT);
                DL.ThisBlockDataCnt = BLOCK_SIZE_DATA_TRANSFER;
            }
            else
            {
                //DL.ThisBlockPacketCnt = DivCeil(DL.TotalDataCnt % BLOCK_SIZE_DATA_TRANSFER, SIZE_ONE_PACKET_CONTENT);
                DL.ThisBlockDataCnt = DL.TotalDataCnt % BLOCK_SIZE_DATA_TRANSFER;
            }
            
        }
        else
        {
            //当前数据块不是最后一个数据块
            //DL.ThisBlockPacketCnt = DivCeil(BLOCK_SIZE_DATA_TRANSFER, SIZE_ONE_PACKET_CONTENT);
            DL.ThisBlockDataCnt = BLOCK_SIZE_DATA_TRANSFER;
        }
        DL.ThisBlockPacketCnt = DivCeil(DL.ThisBlockDataCnt, DL.PacketDataCnt);
    }


    DL.LastPacketTime = GetSystemTime();
    
    return NextBlockIndex;
}

/** 
 * [SetDlPacketDataCnt description]对于WriteSoundBlockData消息中的数据长度进行设定
 * @Author   tin39
 * @DateTime 2019年7月31日T10:20:27+0800
 * @param    DataLength               [description]
 */
void SetDlPacketDataCnt(uint16_t DataLength)
{
    DL.PacketDataCnt = DataLength;
}


int32_t WriteSoundBlockData(uint8_t *buff, uint32_t length)
{
    uint8_t Index;
    int32_t LastLength;
    uint32_t DataLength = length - 1;
    if(DL.IsEnabled == DISABLE)
        return -1;

    Index = buff[0];
    //if(Index == 0)
    //{
    //    //在每个BLOCK的index0时进行计算，得到数据包长度和本快数据包个数
    //    //if(DL.ThisBlockDataCnt == BLOCK_SIZE_DATA_TRANSFER)
    //    if(Index + 1 < DL.ThisBlockPacketCnt)
    //    {
    //        //不是最后一个数据块，数据块大小固定
    //        if(DataLength != SIZE_ONE_PACKET_CONTENT_BLE4 && DataLength != SIZE_ONE_PACKET_CONTENT_BLE42)
    //            return -5;

    //        //得到数据包长度，并计算本块数据包个数
    //        DL.PacketDataCnt = DataLength;
    //        DL.ThisBlockPacketCnt = DivCeil(BLOCK_SIZE_DATA_TRANSFER, DataLength);
    //    }
    //    else
    //    {
    //        //最后一个数据块，数据块大小不固定
    //        if(DataLength != SIZE_ONE_PACKET_CONTENT_BLE4 && DataLength != SIZE_ONE_PACKET_CONTENT_BLE42)
    //        {
    //            //不是标准的数据长度,需要判断当前块内容大小是否和本次数据包一致。一致表示当前块只有一个数据包，否则数据长度有误。
    //            if(DataLength != DL.ThisBlockDataCnt)
    //                return -6;
    //        }

    //        //得到数据包长度，并计算本块数据包个数
    //        DL.PacketDataCnt = DataLength;
    //        DL.ThisBlockPacketCnt = DivCeil(DL.ThisBlockDataCnt, DataLength);
    //    }
    //}

    
    //确认数据包长度合法
    if(Index + 1 < DL.ThisBlockPacketCnt)
    {
        //当前数据包不是块的最后一个数据包，数据包长度固定
        if(DataLength != DL.PacketDataCnt)
            return -2;
    }
    else if(Index + 1 == DL.ThisBlockPacketCnt)
    {
        //当前是本块的最后一个数据包
        if(DL.BlockIndexNow + 1 < DL.TotalBlockCnt)
        {
            //当前块不是最后一块,数据长度固定
            if(DataLength != (BLOCK_SIZE_DATA_TRANSFER % DL.PacketDataCnt))
                return -2;
        }
        else
        {
            //当前块是最后一个块的最后一个数据包，长度需要计算
            LastLength = DL.ThisBlockDataCnt % DL.PacketDataCnt;
            if(LastLength == 0)
                LastLength = DL.PacketDataCnt;
            if(DataLength != LastLength)
                return -3;
        }
        
    }
    else
    { 
        //index溢出
        return -4;
    }




    //将数据拷贝到本数据块的缓冲中
    memcpy(DL.BlockBuff + Index * DL.PacketDataCnt, buff + 1, DataLength);
    DL.PacketReadyBuff[Index] = 1;

    DL.LastPacketTime = GetSystemTime();
    
    return 0;
}



int32_t CheckSoundBlockData(uint8_t BlockIndex, uint8_t *LostPacketIndex, uint32_t CheckSum, bool CheckSumValid)
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
        uint32_t CheckSumCal;

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

            //如果是最后一个数据块，数据不满1K，剩余部分数据用0进行填充，用于后续CheckSum的填充
            for(int k = WriteCnt; k < BLOCK_SIZE_DATA_TRANSFER; k++)
            {
                DL.BlockBuff[k] = 0;
            }
        }

        CheckSumCal = 0;
        for(i = 0; i < BLOCK_SIZE_DATA_TRANSFER / 4; i++)
        {
            uint32_t temp;
            memcpy(&temp, &DL.BlockBuff[i * 4], 4);
            CheckSumCal += temp;
        }

        if(CheckSumValid == false || CheckSumCal == CheckSum)
        {
            //如果不需要检验，或者校验通过，则进行数据的写入。


            
            DataFlashWriteData(DL.ContentAddr + DL.BlockIndexNow * BLOCK_SIZE_DATA_TRANSFER, 
                                          DL.BlockBuff, WriteCnt);
        
            //对当前Block的Flag进行写入
            DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOADER_INDEX_READY_FLAG + DL.BlockIndexNow,
                                              (uint8_t *)&ZeroData, 1);
        }

        
        

/*
        uint32_t CheckCnt = WriteCnt / 4;
        uint32_t CheckData;
        for(i = 0; i < CheckCnt; i++)
        {
            memcpy(&CheckData, DL.BlockBuff + i * 4, 4);
            DL.CheckSum ^= CheckData;
        }
*/
        
    }


    DL.LastPacketTime = GetSystemTime();
        
    return LostPacketCnt;
}

int32_t WriteSoundFinish()
{
    int32_t NextBlockIndex;
    uint8_t DownFinishFlag;
    uint8_t ConvertFinishFlag;
    const uint8_t ZeroData = 0;
    int32_t ret = 0;

    NextBlockIndex = GetNextSoundBlockIndex();
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

    DataFlashReadData(DL.InfoAddr + OFFSET_DOWNLOAD_FINISH_FLAG, 
                                     &DownFinishFlag, LENGTH_DOWNLOAD_FINISH_FLAG);
    DataFlashReadData(DL.InfoAddr + OFFSET_AUDIO_CONVERT_FLAG, 
                                     &ConvertFinishFlag, LENGTH_AUDIO_CONVERT_FLAG);

    if(ConvertFinishFlag == 0)
    {
        //本音源已经完成了转换，直接返回成功
        goto end;
    }

    if(DownFinishFlag == 0xFF)
    {
        //写入音源下载完成的标示
        DataFlashWriteData(DL.InfoAddr + OFFSET_DOWNLOAD_FINISH_FLAG,
                                          (uint8_t *)&ZeroData, LENGTH_DOWNLOAD_FINISH_FLAG);
    }

    


    //Todo
    ret = AudioFileConvert(DL.InfoAddr, DL.TotalDataCnt);
    if(ret != 0)
    {
        goto end;
    }

    DataFlashWriteData(DL.InfoAddr + OFFSET_AUDIO_CONVERT_FLAG,
                                          (uint8_t *)&ZeroData, LENGTH_AUDIO_CONVERT_FLAG);

    ConverFinishTime = GetSystemTime();
    ConverTime = (ConverFinishTime - DlFinishTime) / 10;

    DL.LastPacketTime = GetSystemTime();
end:
    
    DL.IsEnabled = DISABLE;
    return ret;
}

uint8_t ConvertFlag;
void ConvertTest()
{
    DataFlashReadData(DL.InfoAddr + OFFSET_DOWNLOADER_FILE_LENGTH , (uint8_t *)&DL.TotalDataCnt, 4);
    if(ConvertFlag != 0)
        AudioFileConvert(DL.InfoAddr, DL.TotalDataCnt);
}


FunctionalState IsAudioDownloading()
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




