#include "common.h"
#include <stdint.h>
#include <string.h>
#include "tea.h"
#include "engine.h"
#include "DownLoader.h"
#include "RSA.h"
#include "DecryDesKey.h"
#include "SpiFlash.h"
#include "Fs.h"
#include "Mp3Convert.h"
#include "MixerConfig.h"







#define OFFSET_SECRET_TEA					260
#define OFFSET_SECRET_GUID					350


//将下载器中的GUID与音源烧写中的GUID进行比对，若一样则能够烧写，否则返回错误
int ChechAudioGuid(unsigned char *EncyData, uint8_t *GuidDL)
{
    int i;
    unsigned char GuidBuff[LENGHT_DOWNLOADER_GUID];
    unsigned int decryptedLength;
    int status;
    
    for(i = 0; i < 64; i++)
    {
        EncyData[i] = EncyData[i] - 0x4B;
    }

    status = RSAPrivateDecrypt(GuidBuff, &decryptedLength, EncyData, 64, &privateKey);
    if(decryptedLength != 16 || status != 0)
    {
        return 50;
    }

    for(i =0; i < decryptedLength; i++)
    {
        if(GuidBuff[i] != GuidDL[i])
        {
            return 50;
        }
    }
    return 0;
}

int32_t LoadVehicleCfgFromSoundFile(struct FileInfo *fp, uint8_t VehicleIndex)
{
    int16_t AccelerateArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//读取的是大端的数据
    uint16_t ThrottleDownResponseOri;
    uint32_t i;
    AnalyzeMixerConfig(VehicleIndex);
    ReadFile(fp, ACCELERATE_TABLE_OFFSET, (uint8_t *)AccelerateArray, MAX_AUDIO_FILE_NUM_ONE_DIRECT * 2);
    ReadFile(fp, OVERLOAD_FLAG_OFFSET, &engine.OverloadExistFlag, sizeof(engine.OverloadExistFlag));
    ReadFile(fp, ENGINE_DECELERATE_INTERVAL_OFFSET, (uint8_t *)&ThrottleDownResponseOri, sizeof(ThrottleDownResponseOri));


    //音源中的配置以大端进行存储，需要改成小端
    //engine.ThrottleDownResponseOri = ThrottleDownResponseOri >> 8;
    //engine.ThrottleDownResponseOri |= (uint16_t)(ThrottleDownResponseOri << 8);

    for (i = 0; i < MAX_AUDIO_FILE_NUM_ONE_DIRECT; i++)
    {
        engine.AccelerateArrayOri[i] = (uint16_t)AccelerateArray[i] >> 8;
        engine.AccelerateArrayOri[i] |= (int16_t)((uint16_t)AccelerateArray[i] << 8);
    }

    WriteVehicleParamTable(VehicleIndex);
    
    return 0;
}







#define MAX_CNT_CONVERT_FILE			MAX_AUDIO_FILE_NUM_ONE_DIRECT + BACKWARD_AUDIO_CNT + 2
//InfoAddr为下载器的Info的地址
int32_t AudioFileConvert(uint32_t InfoAddr, uint32_t DlTotalSize)
{
    uint32_t i;
    uint32_t SecretAddr = InfoAddr + SPI_FLASH_SECTOR_SIZE;//加密数据的地址
    uint32_t HeadAddr = SecretAddr + 512;//音频文件头的地址
    //uint32_t ContentAddr = HeadAddr + 2048;//音频文件内容的地址
    uint8_t TmpBuff[512];
    int32_t ret = 0;
    uint8_t ForwardAudioFileNum, BackwardAudioFileNum;
    uint16_t StartSector, StopSector;
    uint32_t ForwardDataCntArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//, BackwardDataCntArray[BACKWARD_AUDIO_CNT];
    uint32_t ForwardAudioStartAddrArray[MAX_AUDIO_FILE_NUM_ONE_DIRECT];//, BackwardAudioStartAddrArray[BACKWARD_AUDIO_CNT];
    uint32_t StartDataCnt, StopDataCnt;
    uint32_t StartAudioStartAddr, StopAudioStartAddr;
    bool StopAudioExistFlag = (bool)FALSE;
    struct FileInfo *fp;
    uint8_t VehicleIndex;
    uint8_t ZeroData = 0;
    
    
    
    

    /*************加密区域数据处理***************/
    DataFlashReadData(SecretAddr, TmpBuff, 512);
    //获取音源的TEA密钥
    ret = DecryTeaKey((unsigned char *)(TmpBuff + OFFSET_SECRET_TEA), TeaKeyTable);
    if(ret != 0)
        return ret;
    

    //对音源中的GUID进行校验，确认和下载器中的是否一致
    uint8_t GuidDL[LENGHT_DOWNLOADER_GUID];//下载器中的GUID
    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_GUID, GuidDL, LENGHT_DOWNLOADER_GUID);
    ret = ChechAudioGuid(TmpBuff + OFFSET_SECRET_GUID, GuidDL);
    if(ret != 0)
        return ret;

    
    /***************音频文件头数据处理****************/
    DataFlashReadData(HeadAddr, TmpBuff, 512);
    DecryptContent(TmpBuff, 512);

    //获取前进音频数据 
    ForwardAudioFileNum = TmpBuff[FORWARD_AUDIO_CNT_OFFSET];
    BackwardAudioFileNum = TmpBuff[BACKWARD_AUDIO_CNT_OFFSET];
    if(ForwardAudioFileNum == 0)
    {
        return -10;
    }
    if(ForwardAudioFileNum > MAX_AUDIO_FILE_NUM_ONE_DIRECT || BackwardAudioFileNum > BACKWARD_AUDIO_CNT)
    {
        return -11;
    }

    //获取前进声音的大小和地址
    for(i = 0; i < ForwardAudioFileNum; i++)
    {
        ForwardDataCntArray[i] = TmpBuff[AUDIO_LENGTH_OFFSET + i * 4] << 24;
        ForwardDataCntArray[i] += TmpBuff[AUDIO_LENGTH_OFFSET + i * 4 + 1] << 16;
        ForwardDataCntArray[i] += TmpBuff[AUDIO_LENGTH_OFFSET + i * 4 + 2] << 8;
        ForwardDataCntArray[i] += TmpBuff[AUDIO_LENGTH_OFFSET + i * 4 + 3];

        StartSector = TmpBuff[AUDIO_START_SECTOR_OFFSET + i * 2] << 8;
        StartSector += TmpBuff[AUDIO_START_SECTOR_OFFSET + i * 2 + 1];
        ForwardAudioStartAddrArray[i] = (uint32_t)StartSector * 512; //此地址即为文件内的地址

        if(ForwardDataCntArray[i] == 0 || ForwardDataCntArray[i] >= FS_ONE_FILE_MAX_SIZE ||
            ForwardAudioStartAddrArray[i] == 0 || ForwardAudioStartAddrArray[i] >= FS_SUPPORT_MAX_SIZE)
        {
            return -12;
        }

    }

    //将长度和起始地址信息清空
    memset(TmpBuff + AUDIO_LENGTH_OFFSET, 0xFF, 4 * (ForwardAudioFileNum + BackwardAudioFileNum));
    memset(TmpBuff + AUDIO_START_SECTOR_OFFSET, 0xFF, 2 * (ForwardAudioFileNum + BackwardAudioFileNum));
    

    //不需要对后退声音进行处理，直接抛弃后退声音的数据

    //启动声
    StartDataCnt = TmpBuff[START_SOUND_LENGTH_OFFSET] << 24;
    StartDataCnt += TmpBuff[START_SOUND_LENGTH_OFFSET + 1] << 16;
    StartDataCnt += TmpBuff[START_SOUND_LENGTH_OFFSET + 2] << 8;
    StartDataCnt += TmpBuff[START_SOUND_LENGTH_OFFSET + 3];
    StartSector = TmpBuff[START_SOUND_START_SECTOR_OFFSET] << 8;
    StartSector += TmpBuff[START_SOUND_START_SECTOR_OFFSET + 1];
    StartAudioStartAddr = StartSector * 512;
    if(StartDataCnt == 0 || StartDataCnt >= FS_ONE_FILE_MAX_SIZE ||
            StartAudioStartAddr == 0 || StartAudioStartAddr >= FS_SUPPORT_MAX_SIZE)
    {
        return -13;
    }
    memset(TmpBuff + START_SOUND_LENGTH_OFFSET, 0xFF, 4);
    memset(TmpBuff + START_SOUND_START_SECTOR_OFFSET, 0xFF, 2);

    //熄火声，可以不存在
    StopDataCnt = TmpBuff[STOP_SOUND_LENGTH_OFFSET] << 24;
    StopDataCnt += TmpBuff[STOP_SOUND_LENGTH_OFFSET + 1] << 16;
    StopDataCnt += TmpBuff[STOP_SOUND_LENGTH_OFFSET + 2] << 8;
    StopDataCnt += TmpBuff[STOP_SOUND_LENGTH_OFFSET + 3];
    StopSector = TmpBuff[STOP_SOUND_START_SECTOR_OFFSET] << 8;
    StopSector += TmpBuff[STOP_SOUND_START_SECTOR_OFFSET + 1];
    StopAudioStartAddr = StopSector * 512;
    if(StopDataCnt == 0 || StopDataCnt >= FS_ONE_FILE_MAX_SIZE ||
            StopAudioStartAddr == 0 || StopAudioStartAddr >= FS_SUPPORT_MAX_SIZE)
    {
        StopAudioExistFlag = (bool)FALSE;
    }
    else
    {
        StopAudioExistFlag = (bool)TRUE;
        memset(TmpBuff + STOP_SOUND_LENGTH_OFFSET, 0xFF, 4);
        memset(TmpBuff + STOP_SOUND_START_SECTOR_OFFSET, 0xFF, 2);
    }

    //根据第一个MP3的头部3字节信息，解析音频文件的采样率和码流
    struct Mp3FormatInfo Mp3Format;
    GetAudioMp3Format(HeadAddr + ForwardAudioStartAddrArray[0], ForwardDataCntArray[0], &Mp3Format);
    //将采样率和码流信息写入到音频文件头部
    memcpy(TmpBuff + AUDIO_SAMPLE_RATE_OFFSET, &Mp3Format.SampleRate, AUDIO_SAMPLE_RATE_LENGTH);
    memcpy(TmpBuff + AUDIO_BIT_RATE_OFFSET, &Mp3Format.BitRate, AUDIO_BIT_RATE_LENGTH);
    
    



    //创建一个空的文件
    RecoverBadAudio();
    //DataFlashWriteData(InfoAddr + OFFSET_DOWNLOADER_VEHICLE_INDEX, 
    //                                     &VehicleIndex, LENGTH_DOWNLOADER_VEHICLE_INDEX);
    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_VEHICLE_INDEX, 
                                         &VehicleIndex, LENGTH_DOWNLOADER_VEHICLE_INDEX);
    fp = &File[ENGINE_INDEX(VehicleIndex)];                                     
    ret = CreateEmptyFile(fp);
    if(ret != 0)
    {
        goto Out;
    }

    //写入GUID和SoundName到文件信息中
    ret = DataFlashWriteData(fp->FileIndexAddr + OFFSET_FILE_GUID, GuidDL, SIZE_GUID);
    if(ret != SIZE_GUID)
    {
        ret = -17;
        goto Out;
    }
    uint8_t NameDl[LENGTH_DOWNLOADER_SOUND_NAME_MAX];
    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_SOUND_NAME, NameDl, LENGTH_DOWNLOADER_SOUND_NAME_MAX);
    ret = DataFlashWriteData(fp->FileIndexAddr + OFFSET_FILE_NAME, NameDl, SIZE_SOUND_NAME);
    if(ret != SIZE_SOUND_NAME)
    {
        ret = -35;
        goto Out;
    }
    
    //将文件长度及起始地址信息回填为空，然后写到文件头部
    //文件长度和起始地址已在读取信息处清空
    WriteFileWithAlloc(fp, TmpBuff, 512);
    //剩余头部1.5K空信息写入
    for(i = 0; i < 3; i++)
    {
        DataFlashReadData(HeadAddr + 512 * (i + 1), TmpBuff, 512);
        DecryptContent(TmpBuff, 512);
        WriteFileWithAlloc(fp, TmpBuff, 512);
    }

    /************************音频内容的解密、解码、混淆及写入*****************************/
    uint32_t StartAddrTmp;
    //前进声音处理
    for(i = 0; i < ForwardAudioFileNum; i++)
    {
        //将WAV的当前长度保存
        //WAV的当前长度为当前MP3转换成WAV后的起始地址
        StartAddrTmp = fp->FileLength;
        ret = DecryptionConvertMp3(fp, HeadAddr + ForwardAudioStartAddrArray[i], ForwardDataCntArray[i]);
        if(ret < 0)
        {
            goto Out;
        }
            
        //检查WAV文件没有按照512字节对齐
        if(fp->FileLength % 512 != 0)
        {
            ret = -18;
            goto Out;
        }

        //将数据长度和起始地址信息从MP3信息更新为WAV文件
        ForwardDataCntArray[i] = ret;
        ForwardAudioStartAddrArray[i] =StartAddrTmp;
    }

    //启动声音处理
    //将WAV的当前长度保存
    //WAV的当前长度为当前MP3转换成WAV后的起始地址
    StartAddrTmp = fp->FileLength;
    ret = DecryptionConvertMp3(fp, HeadAddr + StartAudioStartAddr, StartDataCnt);
    if(ret < 0)
    {
        goto Out;
    }

        
    //检查WAV文件没有按照512字节对齐
    if(fp->FileLength % 512 != 0)
    {
        ret = -27;
        goto Out;
    }

    //将数据长度和起始地址信息从MP3信息更新为WAV文件
    StartAudioStartAddr = StartAddrTmp;
    StartDataCnt = ret;

    //熄火声音处理
    if(StopAudioExistFlag == true)
    {
        //将WAV的当前长度保存
        //WAV的当前长度为当前MP3转换成WAV后的起始地址
        StartAddrTmp = fp->FileLength;
        ret = DecryptionConvertMp3(fp, HeadAddr + StopAudioStartAddr, StopDataCnt);
        if(ret < 0)
        {
            goto Out;
        }
            
        //检查WAV文件没有按照512字节对齐
        if(fp->FileLength % 512 != 0)
        {
            ret = -28;
            goto Out;
        }

        //将数据长度和起始地址信息从MP3信息更新为WAV文件
        StopAudioStartAddr = StartAddrTmp;
        StopDataCnt = ret;
    }


    /******************各段声音长度及起始位置写回********************/
    for(i = 0; i < ForwardAudioFileNum; i++)
    {
        TmpBuff[i * 4] = ForwardDataCntArray[i] >> 24;
        TmpBuff[i * 4 + 1] = ForwardDataCntArray[i] >> 16;
        TmpBuff[i * 4 + 2] = ForwardDataCntArray[i] >> 8;
        TmpBuff[i * 4 + 3] = ForwardDataCntArray[i];
    }
    ret = WriteFile(fp, AUDIO_LENGTH_OFFSET, TmpBuff, ForwardAudioFileNum * 4);
    if(ret != ForwardAudioFileNum * 4)
    {
        ret = -29;
        goto Out;
    }

    for(i = 0; i < ForwardAudioFileNum; i++)
    {
        StartSector = ForwardAudioStartAddrArray[i] / 512;
        TmpBuff[i * 2] = StartSector >> 8;
        TmpBuff[i * 2 + 1] = StartSector;
    }
    ret = WriteFile(fp, AUDIO_START_SECTOR_OFFSET, TmpBuff, ForwardAudioFileNum * 2);
    if(ret != ForwardAudioFileNum * 2)
    {
        ret = -30;
        goto Out;
    }

    //由于启动声和熄火声各个字段是连续的，所以可以一次性写入
    uint32_t len;
    TmpBuff[0] = StartDataCnt >> 24;
    TmpBuff[1] = StartDataCnt >> 16;
    TmpBuff[2] = StartDataCnt >> 8;
    TmpBuff[3] = StartDataCnt;
    StartSector = StartAudioStartAddr / 512;
    TmpBuff[4] = StartSector >> 8;
    TmpBuff[5] = StartSector;
    if(StopAudioExistFlag == TRUE)
    {
        TmpBuff[6] = StopDataCnt >> 24;
        TmpBuff[7] = StopDataCnt >> 16;
        TmpBuff[8] = StopDataCnt >> 8;
        TmpBuff[9] = StopDataCnt;
        StartSector = StopAudioStartAddr / 512;
        TmpBuff[10] = StartSector >> 8;
        TmpBuff[11] = StartSector;
        len = 12;
    }
    else
    {
        len = 6;
    }
    ret = WriteFile(fp, START_SOUND_LENGTH_OFFSET, TmpBuff, len);
    if(ret != len)
    {
        ret = -31;
        goto Out;
    }

    //写入文件长度,块中速总数和使能标记写入到文件索引
    //DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_FILE_LENGTH, TmpBuff, LENGTH_DOWNLOADER_FILE_LENGTH);
    ret = DataFlashWriteData(fp->FileIndexAddr + OFFSET_FILE_LENGTH, (uint8_t *)&fp->FileLength, LENGTH_FILE_LENGTH);
    if(ret != LENGTH_FILE_LENGTH)
    {
        ret = -32;
        goto Out;
    }


    ret = DataFlashWriteData(fp->FileIndexAddr + OFFSET_BLOCK_CNT, (uint8_t *)&fp->TotalBlockCnt, LENGTH_BLOCK_CNT);
    if(ret != LENGTH_BLOCK_CNT)
    {
        ret = -33;
        goto Out;
    }

    //在文件完全写入前，将音源中的配置写入到参数表中
    LoadVehicleCfgFromSoundFile(fp, VehicleIndex);
    
    
    ret = DataFlashWriteData(fp->FileIndexAddr + OFFSET_FILE_WHOLE_FLAG, &ZeroData, SIZE_ENABLE_FLAG);
    if(ret != SIZE_ENABLE_FLAG)
    {
        ret = -34;
        goto Out;
    }

    ret = 0;

    
    //InitFile(fp);
Out:
    //音频文件发生了变化
    //对音频文件进行初始化
    InitFile(fp);
    return ret;
}
   

