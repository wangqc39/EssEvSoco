#include "main.h"
#include <string.h>
#include "Downloader.h"


int32_t FirmwareFileConvert(uint32_t InfoAddr, uint32_t DlTotalSize)
{
    uint32_t i;
    uint32_t SecretAddr = InfoAddr + SPI_FLASH_SECTOR_SIZE;//加密数据地址
    uint32_t FirmwareContentAddr = SecretAddr + 512;//音频文件头的地址

    uint8_t TmpBuff[512];
    uint8_t FileType;
    uint32_t FileLength;
    uint32_t CheckSum;
    uint32_t CheckSumCal = 0;
    int32_t ret;
    uint32_t FirmwarePageNum;
    uint32_t ContentLength;
    uint32_t DistAddr;
    uint32_t SourceAddr;
    int32_t RemainCnt;
    FLASH_Status FLASHStatus = FLASH_COMPLETE;
    uint8_t chainCipherBlock[16];
    uint8_t FirmwareContent[16];
    uint32_t FirmwareOneWord;

    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_FILE_TYPE, &FileType, LENGTH_DOWNLOADER_FILE_TYPE);
    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_FILE_LENGTH, (uint8_t *)&FileLength, LENGTH_DOWNLOADER_FILE_LENGTH);
    if(FileType != 1)
        return -1;

    if(FileLength != DlTotalSize)
        return -2;

    if(FileLength < 10 * 1024 || FileLength > 200 * 1024)//固件大小肯定在10K-200K之间
        return -3;

    //文件必然是16字节对齐的
    if(FileLength % 16 != 0)
        return -5;

    DataFlashReadData(SecretAddr, TmpBuff, 512);
    ret = DecryDesKey(TmpBuff + AES_OFFSET, AESKeyTable);
    if(ret != 0)
    {
        return -4;
    }

    

    //文件内容异或校验
    RemainCnt = FileLength;
    SourceAddr = SecretAddr;
    while(RemainCnt > 0)
    {
        DataFlashReadData(SourceAddr, FirmwareContent, 16);
        for(i = 0; i < (16 / sizeof(uint32_t)); i++)
        {
            memcpy(&FirmwareOneWord, FirmwareContent + i * sizeof(uint32_t), sizeof(uint32_t));
            CheckSumCal ^= FirmwareOneWord;
        }
        SourceAddr += 16;
        RemainCnt -= 16;
    }
    DataFlashReadData(InfoAddr + OFFSET_DOWNLOADER_CHECK_SUM, (uint8_t *)&CheckSum, LENGTH_DOWNLOADER_CHECK_SUM);
    if(CheckSumCal != CheckSum)
        return -6;
    
    FLASH_Unlock();
    //对固件区域进行擦除
    ContentLength = FileLength - 512;//512字节为加密部分
    FirmwarePageNum = DivCeil(ContentLength, STM32_PAGE_SIZE);
    for(i = 0; i < FirmwarePageNum && (FLASHStatus == FLASH_COMPLETE); i++)
    {
        FLASHStatus = FLASH_ErasePage(APPLICATION_ADDRESS + (STM32_PAGE_SIZE * i));
    }
    //进行系统信息页的擦除。
    FLASH_ErasePage(SYSTEM_INFO_ADDR);



    //开始进行解密和固件写入
    SourceAddr = FirmwareContentAddr;
    DistAddr = APPLICATION_ADDRESS;
    RemainCnt = ContentLength;
    while(RemainCnt > 0)
    {
        memset(chainCipherBlock, 0, 16);
        aesDecInit();//在执行解密初始化之前可以为AES_Key_Table赋值有效的密码数据
        DataFlashReadData(SourceAddr, FirmwareContent, 16);
        aesDecrypt(FirmwareContent, chainCipherBlock);//AES解密，密文数据存放在dat里面，经解密就能得到之前的明文。
        for(i = 0; i < (16 / sizeof(uint32_t)); i++)
        {
            memcpy(&FirmwareOneWord, FirmwareContent + i * sizeof(uint32_t), sizeof(uint32_t));
            FLASH_ProgramWord(DistAddr + i * sizeof(uint32_t), FirmwareOneWord);
        }
        SourceAddr += 16;
        DistAddr += 16;
        RemainCnt -= 16;
    }
    

    FLASH_ProgramWord(SYSTEM_INFO_ADDR + FIRMWARE_COMPLETE_FLAG_OFFSET, 0);
    FLASH_Lock();
    return 0;
}





