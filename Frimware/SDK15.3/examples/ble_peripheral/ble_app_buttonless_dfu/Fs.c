#include "common.h"
#include "Fs.h"
#include "SystemInfo.h"
#include "DownLoader.h"





//文件索引表
//0-3072字节
#define TABLE_BLOCK_USAGE				0
#define FS_VERSION_OFFSET				499
#define FS_ID_OFFSET						500//FS的标识，写入的是设备ID号，12字节
//文件索引表中内容
#define FS_VERSION						0










//ERROR
#define ERROR_READ_OFFSET				-1
#define ERROR_READ_BLOCK_INDEX			-2
#define ERROR_WRITE_OFFSET				-3


struct FsInfo
{
    unsigned int BlockSize;
    unsigned int BlockCnt;
    unsigned int BlockMask;//0xFFFF0000
    unsigned int BlockOffsetMask;//0x0000FFFF
    unsigned char BlockLeftShiftCnt;//BlockSize对应的左移位数,16
    unsigned char BlockUsageTable[16];
};



struct FsInfo Fs;
struct FileInfo File[FS_SUPPORT_FILE];


int DeleteFile(struct FileInfo *Fp);

//确认文件系统信息
//0:正确
//-1:FS版本号错误
//-2:FS中标识位与设备ID不匹配
int CheckFsInformation()
{
    unsigned char Information[13];
    int i;

    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + FS_VERSION_OFFSET, Information, 13);

    if(Information[0] != FS_VERSION)
    {
        return -1;
    }

    for(i = 0; i < 12; i++)
    {
        if(SystemInformation.DeviceId[i] != Information[1 + i])
        {
            return -2;
        }
    }

    return 0;
}

uint16_t FormatFs()
{
    unsigned char Information[13];
    int i;
    int ret;
    BlockErase(BlockAddr(0));

   
    Information[0] = FS_VERSION;
    for(i = 0; i < 12; i++)
    {
        Information[1 + i] = SystemInformation.DeviceId[i];
    }
    ret = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + FS_VERSION_OFFSET, Information, 13);
    if(ret != 13)
        return 104;

    Information[0] = 0x7F;
    ret = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, Information, 1);
    if(ret != 1)
        return 105;

    return 0;
}



//由于文件号和对应的功能是固定对应的，所以OPEN可被反复执行，并且此FS没有CLOSE
void InitFile(struct FileInfo *Fp)
{
    unsigned char EnableRet;
    int i;
    DataFlashReadData(Fp->FileIndexAddr + OFFSET_FILE_ENABLE_FLAG, &EnableRet, 1);
    if(EnableRet == 0)
    {
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_FILE_LENGTH, (unsigned char *)&Fp->FileLength, LENGTH_FILE_LENGTH);
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&Fp->TotalBlockCnt, LENGTH_BLOCK_CNT);
        if(Fp->FileLength >= FS_ONE_FILE_MAX_SIZE || Fp->TotalBlockCnt >= FS_SUPPORT_MAX_BLOCK_CNT || 
           Fp->FileLength == 0 || Fp->TotalBlockCnt == 0)
        {
            Fp->EnableFlag = DISABLE;
            return;
        }
        Fp->ReadingIndex = 0;
        Fp->WritintIndex = 0;
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(Fp->ReadingIndex), (unsigned char *)&Fp->ReadingBlock, 2);
        Fp->WritingBlock = Fp->ReadingBlock;
        
        for(i = 0; i < FS_ONE_FILE_MAX_SIZE / FS_BLOCK_SIZE; i++)
        {
            Fp->BlockTable[i] = 0;
        }
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(0), (unsigned char *)Fp->BlockTable, 2 * Fp->TotalBlockCnt);
        Fp->EnableFlag = ENABLE;
    }
    else
    {
        Fp->EnableFlag = DISABLE;
    }
}

struct FileInfo *OpenFile(unsigned char FileIndex)
{
    struct FileInfo *fp;
    if(FileIndex >= FS_SUPPORT_FILE)
        return 0;

    fp = &File[FileIndex];
    return fp;
}


//0:正常，-1文件异常
int CheckOneFileWhole(struct FileInfo *Fp)
{
    unsigned char AudioEnableFlag;

    //检查文件系统中音频文件是否存在
    if(Fp->EnableFlag == DISABLE)
    {
        return 0;
    }

    //音频文件存在
    //检查音频文件是否完全写入
    DataFlashReadData(Fp->FileIndexAddr + OFFSET_FILE_WHOLE_FLAG, &AudioEnableFlag, 1);
    if(AudioEnableFlag == 0)
    {
         //音频文件被完全正确写入
         return 0;
    }
    else
    {
        //音频文件未被完全正确写入
        return -1;
    }
}

int RecoverOneAudioFile(struct FileInfo *Fp)
{

    DeleteFile(Fp);
    return 0;
}


int TryRecoverOneAudioFile(struct FileInfo *Fp)
{
    int ret;
    ret = CheckOneFileWhole(Fp);
    if(ret == 0)
    {
       //文件正常
        return 0;
    }

    //文件不正常，进行回收
    RecoverOneAudioFile(Fp);
    return 0;
}

void RecoverBadAudio()
{
    int i;
    for(i = 0; i < FS_SUPPORT_FILE; i++)
    {
        TryRecoverOneAudioFile(&File[i]);
    }
}

void GetFsInfo()
{
    int i;
    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, (unsigned char *)Fs.BlockUsageTable, 16);

    for(i = 0; i < FS_SUPPORT_FILE; i++)
    {
        File[i].FileIndex = i;
        File[i].FileIndexAddr = SectorAddr(SECTOR_FILE(i));
        InitFile(&File[i]);
    }

    RecoverBadAudio();
}

void InitFs()
{
    int i;
    unsigned int BlockSizeTemp;
    int CheckFsRet;
    unsigned char CheckFsFailCnt = 0;
    
    Fs.BlockSize = SpiFlash.FlashBlockSize;
    Fs.BlockCnt = SpiFlash.FlashBlockCnt - SIZE_DOWNLOADER_AREA / Fs.BlockSize;
    //Fs.BlockCnt = SpiFlash.FlashBlockCnt;//for rcplus

    BlockSizeTemp = Fs.BlockSize;
    //i = 0;
    while(BlockSizeTemp != 1)
    {
        BlockSizeTemp = BlockSizeTemp >> 1;
        Fs.BlockLeftShiftCnt++; 
    }
    Fs.BlockMask = 0xFFFFFFFF << Fs.BlockLeftShiftCnt;
    Fs.BlockOffsetMask = 0xFFFFFFFF >> (32 - Fs.BlockLeftShiftCnt);

    for(i = 0; i < 3; i++)
    {
        CheckFsRet = CheckFsInformation();
        if(CheckFsRet != 0)
        {
            CheckFsFailCnt++;
        }
    }

    if(CheckFsFailCnt == 3)
    {
        FormatFs();
    }

    GetFsInfo();
}













int ReadFile(struct FileInfo *Fp, unsigned int offset, unsigned char *buff, unsigned int ReadCnt)
{
    unsigned short ThisReadingIndex;
    unsigned int InBlockOffset;
    unsigned int ReadAddr;
    unsigned int ThisReadCnt;
    //确认读取的偏移量是否在文件内
    if(offset >= Fp->FileLength)
    {
        return ERROR_READ_OFFSET;
    }
    
    //确认读取的所有数据是否均在文件长度内，否则给读取长度设定一个合适的值
    if(offset + ReadCnt > Fp->FileLength)
    {
        ReadCnt = Fp->FileLength - offset;
    }

    while(ReadCnt > 0)
    {
        //获取读取的数据在block内的偏移量
        InBlockOffset = offset & Fs.BlockOffsetMask;
        if(InBlockOffset + ReadCnt > Fs.BlockSize)
        {
            ThisReadCnt = Fs.BlockSize - InBlockOffset;
        }
        else
        {
            ThisReadCnt = ReadCnt;
        }

        //获取读取的block偏移量
        ThisReadingIndex = offset >> Fs.BlockLeftShiftCnt;
        if(ThisReadingIndex == Fp->ReadingIndex)
        {
            //和上次读取的block是同一个block
            ReadAddr = BlockAddr(Fp->ReadingBlock) + InBlockOffset;
        }
        else
        {
            //和上一次读取的不是同一个block，通过索引读取block绝对block号
            Fp->ReadingIndex = ThisReadingIndex;
            DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(ThisReadingIndex), (unsigned char *)&Fp->ReadingBlock, 2);
            if(Fp->ReadingBlock > Fs.BlockCnt)
            {
                return ERROR_READ_BLOCK_INDEX;
            }
            ReadAddr = BlockAddr(Fp->ReadingBlock) + InBlockOffset;
        }
        DataFlashReadData(ReadAddr, buff, ThisReadCnt);
        ReadCnt -= ThisReadCnt;
        buff += ThisReadCnt;
        offset += ThisReadCnt;
    }


    return ReadCnt;
    
}




//写入的空间在64Kblock内，超出写入会有异常
int WriteFile(struct FileInfo *Fp, unsigned int offset, unsigned char *buff, unsigned int WriteCnt)
{
    unsigned short ThisWritingIndex;
    unsigned int InBlockOffset;
    unsigned int WriteAddr;
    int ret;
    //确认读取的偏移量是否在文件内
    if(offset >= Fp->FileLength)
    {
        return ERROR_WRITE_OFFSET;
    }
    //确认读取的所有数据是否均在文件长度内，否则给读取长度设定一个合适的值
    if(offset + WriteCnt > Fp->FileLength)
    {
        WriteCnt = Fp->FileLength - offset;
    }
    //获取读取的数据在block内的偏移量
    InBlockOffset = offset & Fs.BlockOffsetMask;
    //获取读取的block偏移量
    ThisWritingIndex = offset >> Fs.BlockLeftShiftCnt;
    if(ThisWritingIndex == Fp->WritintIndex)
    {
        //和上次读取的block是同一个block
        WriteAddr = BlockAddr(Fp->WritingBlock) + InBlockOffset;
    }
    else
    {
        Fp->WritintIndex = ThisWritingIndex;
        //和上一次读取的不是同一个block，通过索引读取block绝对block号
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(ThisWritingIndex), (unsigned char *)&Fp->WritingBlock, 2);
        if(Fp->WritingBlock > Fs.BlockCnt)
        {
            return ERROR_READ_BLOCK_INDEX;
        }
        WriteAddr = BlockAddr(Fp->WritingBlock) + InBlockOffset;
    }
    ret = DataFlashWriteData(WriteAddr, buff, WriteCnt);

    return ret;
}

//进行音频文件的删除
//回收使用的block，对音频文件的索引扇区进行擦除
//对回收的block不进行擦除
int DeleteFile(struct FileInfo *Fp)
{
    unsigned char BlockUsageTable[FS_SUPPORT_MAX_BLOCK_CNT / 8];
    int i;
    unsigned short int BlockIndex;
    unsigned short int TableOffset;
    unsigned char ShiftCnt;
    //unsigned int FileBlockCnt;
    int WriteRet;
    
    Fp->EnableFlag = DISABLE;
//    DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&FileBlockCnt, LENGTH_BLOCK_CNT);
//    if(FileBlockCnt >= Fs.BlockCnt)
//    {
//        FileBlockCnt = 0;
//    }

//    //读取Block使用情况列表
//    if(FileBlockCnt != 0)
//    {
//        //回收文件占用的block
//        DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
//        //原有的文件占用了一定的block空间，进行这些block的移除,空间释放
//        for(i = 0; i < FileBlockCnt; i++)
//        {
//            //读取文件内容的索引，并将BLOCK使用情况表中的响应位进行清除
//            DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&BlockIndex, 2);
//            if(BlockIndex < Fs.BlockCnt)
//            {
//                TableOffset = BlockIndex >> 3;
//                ShiftCnt = BlockIndex & 7;
//                ShiftCnt = 7 - ShiftCnt;
//                BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
//                Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
//            }
//        }
//        WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
//    }

    //由于block的动态申请及分配，所以可能Block表已占用，但是在文件的block数量的字段信息上仍未被写入
    //所以在回收文件时，需要对文件整个的BLOCK使用表进行遍历
    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    //原有的文件占用了一定的block空间，进行这些block的移除,空间释放
    i = 0;
    do
    {
        //读取文件内容的索引，并将BLOCK使用情况表中的响应位进行清除
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&BlockIndex, 2);
        if(BlockIndex < Fs.BlockCnt)
        {
            TableOffset = BlockIndex >> 3;
            ShiftCnt = BlockIndex & 7;
            ShiftCnt = 7 - ShiftCnt;
            BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
            Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
        }
        i++;
    }while(BlockIndex != 0xFFFF && i < MAX_BLOCK_INDEX_CNT);
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);

    SectorErase(Fp->FileIndexAddr);
    return WriteRet;
}












int GetFsFreeBlockCnt()
{
    int i;
    int BitCnt;
    int EmptyBlock = 0;

    unsigned char BlockUsageTable[FS_SUPPORT_MAX_BLOCK_CNT / 8];

    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);

    for(i = 0; i < Fs.BlockCnt / 8; i++)
    {
        Fs.BlockUsageTable[i] = BlockUsageTable[i];
        for(BitCnt = 0; BitCnt < 8; BitCnt++)
        {
            if(((BlockUsageTable[i] >> BitCnt) & 0x01) != 0)
            {
                EmptyBlock++;
            }
        }
    }

    return EmptyBlock;
}

//0:busy, 1:free, -1:error
int CheckBlockFree(unsigned char *BlockUsageTable, unsigned short int BlockIndex)
{
    unsigned short int TableOffset;
    unsigned char ShiftCnt;
    if(BlockIndex >= Fs.BlockCnt)
        return -1;

    TableOffset = BlockIndex >> 3;
    ShiftCnt = BlockIndex & 7;
    ShiftCnt = 7 - ShiftCnt;
    if((BlockUsageTable[TableOffset] & (1 << ShiftCnt)) == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

//0:未找到空闲的block, 
//其他:空闲的block号
unsigned short SearchFreeBlock(unsigned char *BlockUsageTable, unsigned short int StartBlockIndex)
{
    int ret;

    while(1)
    {
        ret = CheckBlockFree(BlockUsageTable, StartBlockIndex);
        if(ret == -1)
        {
            return 0;
        }
        else if(ret == 0)
        {
            StartBlockIndex++;
        }
        else if(ret == 1)
        {
            return StartBlockIndex;
        }
        else
        {
            return 0;
        }
    }
}

//所有对于FAT表中block使用情况和文件索引中block索引仅在本函数中进行操作
uint16_t AllocBlockToFile(struct FileInfo *Fp, unsigned int NeedBlockCnt, unsigned int FileLength)
{
    unsigned char BlockUsageTable[FS_SUPPORT_MAX_BLOCK_CNT / 8];
    int i;
    unsigned short int BlockIndex;
    unsigned short int TableOffset;
    unsigned char ShiftCnt;
    unsigned int FileBlockCnt;

    unsigned short int ThisBlockIndex;
    //int ret;
    unsigned char ZeroData;
    int WriteRet;
    uint16_t ErrorCnt = 0;

    Fp->EnableFlag = DISABLE;
    //读取block使用表
    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    //读取文件原来占用的block数量
    DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&FileBlockCnt, LENGTH_BLOCK_CNT);
    if(FileBlockCnt >= Fs.BlockCnt)
    {
        FileBlockCnt = 0;
    }

    //若文件原先占用了block，则进行回收
    if(FileBlockCnt != 0)
    {
        //回收文件占用的block
        //原有的文件占用了一定的block空间，进行这些block的移除,空间释放
        for(i = 0; i < FileBlockCnt; i++)
        {
            //读取文件内容的索引，并将BLOCK使用情况表中的响应位进行清除
            DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&BlockIndex, 2);
            if(BlockIndex < Fs.BlockCnt)
            {
                TableOffset = BlockIndex >> 3;
                ShiftCnt = BlockIndex & 7;
                ShiftCnt = 7 - ShiftCnt;
                BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
                Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
            }
        }
    }

    SectorErase(Fp->FileIndexAddr);

    ThisBlockIndex = 0;
    //进行空闲block的检索及分配及擦除
    for(i = 0; i < NeedBlockCnt; i++)
    {
        IWDG_ReloadCounter();
        ThisBlockIndex = SearchFreeBlock(BlockUsageTable, ThisBlockIndex);
        if(ThisBlockIndex == 0)//未找到空闲的块
            break;

        //写入索引号
        WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&ThisBlockIndex, 2);
        if(WriteRet != 2)
        {
            ErrorCnt++;
        }
        Fp->BlockTable[i] = ThisBlockIndex;
        //分配后立即擦除block
        BlockErase(BlockAddr(ThisBlockIndex));
        //在block表中将该block标记为已使用
        TableOffset = ThisBlockIndex >> 3;
        ShiftCnt = ThisBlockIndex & 7;
        ShiftCnt = 7 - ShiftCnt;
        BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] & ~(1 << ShiftCnt);
        Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] & ~(1 << ShiftCnt);
    }

    //写入文件索引的相关信息
    WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_FILE_LENGTH, (unsigned char *)&FileLength, LENGTH_FILE_LENGTH);
    if(WriteRet != LENGTH_FILE_LENGTH)
    {
        ErrorCnt++;
    }
    WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&i, LENGTH_BLOCK_CNT);
    if(WriteRet != LENGTH_BLOCK_CNT)
    {
        ErrorCnt++;
    }
    ZeroData = 0;
    WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_FILE_ENABLE_FLAG, &ZeroData, 1);
    if(WriteRet != 1)
    {
        ErrorCnt++;
    }

    //写入block表
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    if(WriteRet != FS_SUPPORT_MAX_BLOCK_CNT / 8)
    {
        ErrorCnt++;
    }


    InitFile(Fp);
    if(ErrorCnt == 0)
    {
        return 0;
    }
    else
    {
        return 200 + ErrorCnt;
    }
}


int CreateFile(struct FileInfo *Fp, unsigned int FileLength)
{
    unsigned int TotalFreeBlock = 0;
    unsigned int NeedBlockCnt;
    unsigned int FileBlockCnt;
    //unsigned char IsFileHaveBlock = 1;//用来标识，本文件是否有占用FAT表中的block
    /*if(Fp->EnableFlag == DISABLE)
    {
        TotalFreeBlock = GetFsFreeBlockCnt();
    }
    else
    {
        TotalFreeBlock = GetFsFreeBlockCnt() + Fp->TotalBlockCnt;
    }*/
    //无论文件是否enable均去flash读取占用的block数量
    //防止在下载声音过程中断开，导致文件DISABLE但是BLOCK被占用的情况
    DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&FileBlockCnt, LENGTH_BLOCK_CNT);
    if(FileBlockCnt >= Fs.BlockCnt)
    {
        FileBlockCnt = 0;
    }
    TotalFreeBlock = GetFsFreeBlockCnt() + FileBlockCnt;

    NeedBlockCnt = (FileLength >> Fs.BlockLeftShiftCnt);// + 1;
    if((FileLength & (0xFFFFFFFF >> Fs.BlockLeftShiftCnt)) != 0)
    {
        NeedBlockCnt++;
    }
    if(NeedBlockCnt > TotalFreeBlock)
    {
        return -1;
    }

    if(AllocBlockToFile(Fp, NeedBlockCnt, FileLength) != 0)
        return -1;
        
    return 0;
}

//回收原先文件的Block
//创建后仅写入OFFSET_FILE_ENABLE_FLAG信息
int32_t CreateEmptyFile(struct FileInfo *Fp)
{
    unsigned char BlockUsageTable[FS_SUPPORT_MAX_BLOCK_CNT / 8];
    int i;
    unsigned short int BlockIndex;
    unsigned short int TableOffset;
    unsigned char ShiftCnt;
    //unsigned int FileBlockCnt;

    //unsigned short int ThisBlockIndex;
    //int ret;
    unsigned char ZeroData;
    int WriteRet;
    //uint16_t ErrorCnt = 0;

    Fp->EnableFlag = DISABLE;
    
//    //读取文件原来占用的block数量
//    DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_CNT, (unsigned char *)&FileBlockCnt, LENGTH_BLOCK_CNT);
//    if(FileBlockCnt >= Fs.BlockCnt)
//    {
//        FileBlockCnt = 0;
//    }

//    //若文件原先占用了block，则进行回收
//    if(FileBlockCnt != 0)
//    {
//        //读取block使用表
//        DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
//    
//        //回收文件占用的block
//        //原有的文件占用了一定的block空间，进行这些block的移除,空间释放
//        for(i = 0; i < FileBlockCnt; i++)
//        {
//            //读取文件内容的索引，并将BLOCK使用情况表中的响应位进行清除
//            DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&BlockIndex, 2);
//            if(BlockIndex < Fs.BlockCnt)
//            {
//                TableOffset = BlockIndex >> 3;
//                ShiftCnt = BlockIndex & 7;
//                ShiftCnt = 7 - ShiftCnt;
//                BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
//                Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
//            }
//        }

//        //写入block表
//        WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
//        if(WriteRet != FS_SUPPORT_MAX_BLOCK_CNT / 8)
//        {
//            return -15;
//        }
//    }

    //由于block的动态申请及分配，所以可能Block表已占用，但是在文件的block数量的字段信息上仍未被写入
    //所以在回收文件时，需要对文件整个的BLOCK使用表进行遍历
    DataFlashReadData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    //原有的文件占用了一定的block空间，进行这些block的移除,空间释放
    i = 0;
    do
    {
        //读取文件内容的索引，并将BLOCK使用情况表中的响应位进行清除
        DataFlashReadData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(i), (unsigned char *)&BlockIndex, 2);
        if(BlockIndex < Fs.BlockCnt)
        {
            TableOffset = BlockIndex >> 3;
            ShiftCnt = BlockIndex & 7;
            ShiftCnt = 7 - ShiftCnt;
            BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
            Fs.BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] | (1 << ShiftCnt);
        }
        i++;
    }while(BlockIndex != 0xFFFF && i < MAX_BLOCK_INDEX_CNT);
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    if(WriteRet != FS_SUPPORT_MAX_BLOCK_CNT / 8)
    {
        return -15;
    }
    

    //删除文件索引的信息
    SectorErase(Fp->FileIndexAddr);

   

    //写入文件索引的相关信息
    ZeroData = 0;
    WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_FILE_ENABLE_FLAG, &ZeroData, 1);
    if(WriteRet != 1)
    {
        return -14;
    }

    Fp->FileLength = 0;
    Fp->WritintIndex = 0;
    Fp->TotalBlockCnt = 0;
    Fp->FreeDataCntInBlock = 0;
    

    

    return 0;
}









unsigned int  GetFileLength(struct FileInfo *Fp)
{
    unsigned int length;
    DataFlashReadData(Fp->FileIndexAddr + OFFSET_FILE_LENGTH, (unsigned char *)&length, LENGTH_FILE_LENGTH);
    if(length >= SpiFlash.FlashSize)
    {
        length = 0;
    }
    return length;
}


unsigned int GetFsSize()
{
    return ((Fs.BlockCnt - 1) * Fs.BlockSize);
}


unsigned int GetFsEmptySize()
{
    return (GetFsFreeBlockCnt() * Fs.BlockSize);
}


/*void ReadSoundConfig(unsigned char SoundIndex, unsigned int offset, unsigned char *buff, unsigned int length)
{
    DataFlashReadData(SectorAddr(SECTOR_SOUND_CONFIG(SoundIndex)) + offset, buff, length);
}

#define MIXER_CONFIG_SIZE			512
int WriteSoundConfig(unsigned char SoundIndex, unsigned int offset, unsigned char *buff, unsigned int length)
{
    int WriteRet;
    if(offset + length > MIXER_CONFIG_SIZE)
        return -1;

        
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_SOUND_CONFIG(SoundIndex)) + offset, buff, length);
    return WriteRet;
}



void ReadSystemConfig(unsigned int offset, unsigned char *buff, unsigned int length)
{
    DataFlashReadData(SectorAddr(SECTOR_SYSTEM_CONFIG) + offset, buff, length);
}

#define SYSTEM_CONFIG_SIZE				512
int WriteSystemConfig(unsigned int offset, unsigned char *buff, unsigned int length)
{
    int WriteRet;
    if(offset + length > SYSTEM_CONFIG_SIZE)
        return -1;
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_SYSTEM_CONFIG) + offset, buff, length);
    return WriteRet;
}*/

int32_t WriteParamSector(uint8_t *ParamTable, uint32_t ParamAddr)
{
    if(ParamAddr % SPI_FLASH_SECTOR_SIZE != 0)
        return -1;

    if(ParamAddr > SectorAddr((1 + VEHICLE_CNT)))
        return -2;

    SectorErase(ParamAddr);
    DataFlashDirectWriteData(ParamAddr, ParamTable, SPI_FLASH_SECTOR_SIZE);
    //DataFlashWriteData(ParamAddr, ParamTable, SPI_FLASH_SECTOR_SIZE);

    return 0;
}

int32_t AllocOneFreeBlock(struct FileInfo *Fp)
{
    unsigned short int TableOffset;
    unsigned char ShiftCnt;
    //unsigned int FileBlockCnt;

    unsigned short int ThisBlockIndex;//Block在文件系统中的索引号
    int WriteRet;

    
    //进行空闲block的检索及分配及擦除
    IWDG_ReloadCounter();
    ThisBlockIndex = 0;
    ThisBlockIndex = SearchFreeBlock(Fs.BlockUsageTable, ThisBlockIndex);
    if(ThisBlockIndex == 0)//未找到空闲的块
        return -20;

    //写入索引号
    WriteRet = DataFlashWriteData(Fp->FileIndexAddr + OFFSET_BLOCK_INDEX(Fp->TotalBlockCnt), (unsigned char *)&ThisBlockIndex, 2);
    if(WriteRet != 2)
    {
        return -21;
    }
    Fp->BlockTable[Fp->TotalBlockCnt] = ThisBlockIndex;
    //分配后立即擦除block
    BlockErase(BlockAddr(ThisBlockIndex));
    //在block表中将该block标记为已使用
    TableOffset = ThisBlockIndex >> 3;
    ShiftCnt = ThisBlockIndex & 7;
    ShiftCnt = 7 - ShiftCnt;
    //BlockUsageTable[TableOffset] = BlockUsageTable[TableOffset] & ~(1 << ShiftCnt);
    Fs.BlockUsageTable[TableOffset] = Fs.BlockUsageTable[TableOffset] & ~(1 << ShiftCnt);

    //写入block表
    WriteRet = DataFlashWriteData(SectorAddr(SECTOR_FAT_TABLE) + TABLE_BLOCK_USAGE, Fs.BlockUsageTable, FS_SUPPORT_MAX_BLOCK_CNT / 8);
    if(WriteRet != FS_SUPPORT_MAX_BLOCK_CNT / 8)
    {
        return -22;
    }

    //分配成功TotalBlockCnt增加1
    Fp->TotalBlockCnt++;
    Fp->WritintIndex = Fp->TotalBlockCnt - 1;//++;
    return ThisBlockIndex;
}

//写入操作不再支持指定offset
//offset随着数据的写入自动增加
//写入过程中，Fp->FileLength表示offset
int WriteFileWithAlloc(struct FileInfo *Fp, unsigned char *buff, unsigned int WriteCnt)
{
    uint32_t ThisWriteCnt;
    int32_t ret;
    unsigned int InBlockOffset;
    unsigned short ThisWritingIndex;
    uint32_t WrittenDataCnt = 0;
    uint32_t WriteAddr;

    //可以跨块写入，跨块则分2次写入
    while(WriteCnt > 0)
    {
        //如果数据块内剩余空间为0，进行空闲Block的申请
        if(Fp->FreeDataCntInBlock == 0)
        {
            //块内空闲数据空，申请一个空的数据块
            ret = AllocOneFreeBlock(Fp);
            if(ret < 0)
                return ret;

            ThisWritingIndex = ret;
            //将申请到的数据块赋值给当前正在写的数据块
            Fp->WritingBlock = ThisWritingIndex;
            Fp->FreeDataCntInBlock = WINBOND_FLASH_BLOCK_SIZE;
        }

        if(WriteCnt > Fp->FreeDataCntInBlock)
        {
            //本数据块空余不够本次数据写入，需要重新申请空闲的数据块
            ThisWriteCnt = Fp->FreeDataCntInBlock;
        }
        else
        {
            //块中空闲空间足够，直接写入
            ThisWriteCnt = WriteCnt;
        }

        //获取读取的数据在block内的偏移量
        InBlockOffset = Fp->FileLength & Fs.BlockOffsetMask;
        //获取写入的绝对物理地址
        WriteAddr = BlockAddr(Fp->WritingBlock) + InBlockOffset;
        ret = DataFlashWriteData(WriteAddr, buff, ThisWriteCnt);
        if(ret != ThisWriteCnt)
        {
            return ret;
        }

        WriteCnt -= ThisWriteCnt;
        buff += ThisWriteCnt;
        Fp->FileLength += ThisWriteCnt;
        Fp->FreeDataCntInBlock -= ThisWriteCnt;
        WrittenDataCnt += ThisWriteCnt;
    }

    return WrittenDataCnt;
}

//int32_t f_read (
//	struct FileInfo *Fp, 		/* Pointer to the file object */
//	void *buff,		/* Pointer to data buffer */
//	UINT btr,		/* Number of bytes to read */
//	UINT *br		/* Pointer to number of bytes read */
//)
//{
//    uint32_t ReadCnt;
//    ReadCnt = ReadFile(Fp, Fp->ReadSeek, buff, btr);
//    Fp->ReadSeek += ReadCnt;
//    *br = ReadCnt;

//    return 0;
//}

//int32_t f_seek(
//	struct FileInfo *Fp, 	/* Pointer to the file object */
//	uint32_t ofs		/* File pointer from top of file */
//)
//{
//    if(ofs >= Fp->FileLength)
//        return -1;

//    Fp->ReadSeek = ofs;
//    return 0;
//}


