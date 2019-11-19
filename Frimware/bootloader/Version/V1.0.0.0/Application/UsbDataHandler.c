#include "main.h"
#include "OnChipLed.h"

//传输层宏定义
#define START_BIT_CONTENT1		0xF1
#define START_BIT_CONTENT2		0x0E





//struct UsbSerialTransLayerInfo UsbSerialTransLayer;

FunctionalState MassStorageWriteEnableFlag;

void UsbSerialSendResponseDirectly(unsigned char *buff, unsigned short int cnt)
{
    unsigned char CheckByte;
    int i;
    unsigned char *ptr; 
    ptr = com.HandleAndTxBuff;
    *ptr++ = START_BIT_CONTENT1;
    *ptr++ = START_BIT_CONTENT2;
    *ptr++ = (unsigned char)((cnt + 6) >> 8);
    *ptr++ = (unsigned char)(cnt + 6);
    for(i = 0; i < cnt; i++)
    {
        *ptr++ = buff[i];
    }
    *ptr++ = com.LastMessageId;

    ptr = com.HandleAndTxBuff + 2;
    CheckByte = 0xFF;
    for(i = 2; i < cnt + 5; i++)
    {
        CheckByte ^= *ptr;
        ptr++;
    }
    com.HandleAndTxBuff[cnt + 5] = CheckByte;
    
    ComSendData(cnt + 6);
}


void UsbSerialSendResponse(unsigned char *buff, unsigned short int cnt)
{
    UsbSerialSendResponseDirectly(buff, cnt);
}


void UsbSerialSendErrorPacket(unsigned short int ErrorId)
{
    unsigned char SendBuffer[3];
    SendBuffer[0] = ERROR_CMD;
    SendBuffer[1] = (unsigned char)(ErrorId >> 8);
    SendBuffer[2] = (unsigned char)(ErrorId & 0xff);
    UsbSerialSendResponse(SendBuffer, 3);
}


unsigned int FileLength;
unsigned int NbrOfPage;
u16 WriteApplicationDataStartCmdHandler(unsigned char *buff, unsigned short int cnt)
{
    unsigned char SendBuffer[1];
    unsigned char SystemInfoBuff[STM32_PAGE_SIZE];
    //unsigned int FileLength;
    //unsigned int NbrOfPage;
    unsigned int i;
    u32 WriteData;
    FLASH_Status FLASHStatus = FLASH_COMPLETE;

    FLASH_Unlock();
    
    FileLength = (buff[0] << 24) | (buff[1] << 16) | (buff[2] << 8) | buff[3];
    //头部为DES密钥区域，剪掉该长度
    FileLength -= 512;


    //检查文件长度，若文件过长返回错误终止下载，防止覆盖加密数据区域
    if(FileLength > OnChipAudioMenuAddr - APPLICATION_ADDRESS)
        return 50;

    //进行固件标志位的擦除
    for(i = 0; i < STM32_PAGE_SIZE; i++)
    {
        SystemInfoBuff[i] = *(unsigned char *)(SYSTEM_INFO_ADDR + i);
    }
    if(FLASH_ErasePage(SYSTEM_INFO_ADDR) != FLASH_COMPLETE)
    {
        return 68;
    }
    SystemInfoBuff[FIRMWARE_COMPLETE_FLAG_OFFSET] = FIRMWARE_BROKEN;
    for(i = 0; i < STM32_PAGE_SIZE / 4; i++)
    {
        WriteData = (u32)((u32)SystemInfoBuff[i * 4] | SystemInfoBuff[i * 4 + 1] << 8  | SystemInfoBuff[i * 4 + 2] << 16 | SystemInfoBuff[i * 4 + 3] << 24);
        if(FLASH_ProgramWord(SYSTEM_INFO_ADDR + i * 4, WriteData) != FLASH_COMPLETE)
	 {
	     return 69;
	 }
    }	    

    OnChipLedStatus = RC_PLUS_DOWNLOADING;
    
    
    NbrOfPage = OnChipFlashPagesMask(FileLength);
    for(i = 0; i < NbrOfPage && (FLASHStatus == FLASH_COMPLETE); i++)
    {
        FLASHStatus = FLASH_ErasePage(APPLICATION_ADDRESS + (STM32_PAGE_SIZE * i));
    }

    
    SendBuffer[0] = WRITE_APPLICATION_DATA_START_CMD + 0x80;
    UsbSerialSendResponse(SendBuffer, 1);
    return 0;
}





int WriteApplicationWithEnpt(unsigned short int BlockCnt, unsigned char *buff)
{
    unsigned char chainCipherBlock[16];
    int i, j, k;
    int ret;
    unsigned int WriteData;
    if(BlockCnt == 0)
    {
        /*for(i = 0; i < 512; i++)
        {
            FirstBuff[i] = buff[2 + i];
        }
        DecryDesKey((unsigned char *)(FirstBuff + 75), AESKeyTable);*/
        //2字节为开头的block编号
        ret = DecryDesKey((unsigned char *)(buff + AES_OFFSET + 2), AESKeyTable);
        if(ret != 0)
        {
            return 100;
        }
    }
    else
    {
        //头部扇区存放DES的密钥，此处将扇区数减1
        BlockCnt--;
        for(i = 0; i < 32; i++)
        {
            
           
                for(k = 0; k < 16; k++)
                {
                    chainCipherBlock[k] = 0;
                }
    
        	     aesDecInit();//在执行解密初始化之前可以为AES_Key_Table赋值有效的密码数据
        
        	     aesDecrypt(buff + 2 + i * 16, chainCipherBlock);//AES解密，密文数据存放在dat里面，经解密就能得到之前的明文。
    
                /*if(BlockCnt == 0 && i ==0)
                {
                    for(m = 0; m < 16; m++)
                    {
                        SecondBuff[m] = buff[2 + m];
                    }
                    if(SecondBuff[3] != 0x20)
                    {
                        return 0;
                    }
                    
                }*/
    
                for(j = 0; j < 4; j++)
                {
                    WriteData = buff[2 + i * 16 + j * 4];
                    WriteData += (unsigned int)((unsigned int)buff[2 + i * 16 + j * 4 + 1] << 8);
                    WriteData += (unsigned int)((unsigned int)buff[2 + i * 16 + j * 4 + 2] << 16);
                    WriteData += (unsigned int)((unsigned int)buff[2 + i * 16 + j * 4 + 3] << 24);
                    FLASH_ProgramWord(APPLICATION_ADDRESS + BlockCnt * 512 + i * 16 + j * 4, 
                                                           WriteData);
                }
                //RemainFileLength -= 16;
        }
    }
    return 0;
}

int WriteApplicationWithoutEnpt(unsigned short int BlockCnt, unsigned char *buff)
{
    int i, j;
    for(i = 0; i < 32; i++)
    {
        for(j = 0; j < 4; j++)
        {
            FLASH_ProgramWord(APPLICATION_ADDRESS + BlockCnt * 512 + i * 16 + j * 4, 
                                                           *(unsigned int*)(buff + 2 + i * 16 + j * 4));
        }
    }
    return 0;
}



u16 WriteApplicationDataCmdHandler(unsigned char *buff, unsigned short int cnt)
{
    unsigned char SendBuff[1];
    unsigned short int BlockCnt;
    volatile int i;
    int ret;
    
    static FunctionalState EnptFlag = DISABLE;
    unsigned char *DataPtr;
    BlockCnt = buff[0] * 256 + buff[1];
    //其中1扇区为固件头部的DES密钥区域
    if(BlockCnt > ((OnChipAudioMenuAddr - APPLICATION_ADDRESS) / 512 + 1))
        return 10;

    if(cnt != 512 + 2)
        return 11;

    //BlockCntNow = BlockCnt;


    if(BlockCnt == 0)
    {
        //若RSA密钥区连续32字节均为0xFF，则表示没有RSA密钥
        EnptFlag = DISABLE;
        DataPtr = (unsigned char *)(SECURE_START_ADDR);
        for(i = 0; i < 32; i++)
        {
            if(*DataPtr != 0xFF)
            {
                EnptFlag = ENABLE;
                break;
            }
            DataPtr++;
        }
    }

    if(EnptFlag == DISABLE)
    {
        ret = WriteApplicationWithoutEnpt(BlockCnt, buff);
    }
    else
    {
       ret =  WriteApplicationWithEnpt(BlockCnt, buff);
       if(ret != 0)
       {
           return ret;
       }
    }

    
    
    


    

    
    
    SendBuff[0] = WRITE_APPLICATION_DATA_CMD + 0x80;
    UsbSerialSendResponse(SendBuff, 1);
    return 0;
}

static void WriteApplicationDataFinishCmdHandler()
{
    unsigned char SendBuffer[1];
    FLASH_ProgramWord(SYSTEM_INFO_ADDR + FIRMWARE_COMPLETE_FLAG_OFFSET, 0xFFFFFF00);
    FLASH_Lock();

    OnChipLedStatus = RC_PLUS_CONNECT;

    SendBuffer[0] = WRITE_APPLICATION_DATA_FINISH_CMD + 0x80;
    UsbSerialSendResponse(SendBuffer, 1);

    mDelay(1000);
    RunToApplication(APPLICATION_ADDRESS);
}

static void RunToApplicationCmdHandler()
{
    unsigned char SendBuffer[1];


    SendBuffer[0] = RUN_TO_APPLICATION + 0x80;
    UsbSerialSendResponse(SendBuffer, 1);
    mDelay(1000);
    RunToApplication(APPLICATION_ADDRESS);
}


void CheckWorkingProgrammeCmdHandler()
{
    unsigned char SendBuffer[2];


    SendBuffer[0] = CHECK_WORKING_PROGRAMME_CMD + 0x80;
    //0:bootlaoder; 1:FrogApp
    SendBuffer[1] = 0;
    UsbSerialSendResponse(SendBuffer, 2);
}


void ReadSerialNumCmdHandler()
{
    vu32 ID0, ID2, ID1;
    unsigned char SendBuffer[35];
    char HwVersionBuff[16] = ESS_ONE_PLUS_HARDWARE_VERSION;
    int i;
    ID0 = *(vu32*)(ID0_ADDR);
    ID1 = *(vu32*)(ID1_ADDR);
    ID2 = *(vu32*)(ID2_ADDR);

    //异或为可逆处理，同时又能保证异或后的数据仍旧是全球唯一的
    ID0 = ID0 ^ 0x55aaaa55;
    ID1 = ID1 ^ 0xaa5555aa;;
    ID2 = ID2 ^ 0x669977ee;


    SendBuffer[0] = READ_SERIAL_NUM_CMD + 0x80;
    SendBuffer[1] = (u8)(ID2 >> 24);
    SendBuffer[2] = (u8)(ID1 >> 24);
    SendBuffer[3] = (u8)(ID0 >> 24);
    SendBuffer[4] = (u8)(ID0 >> 16);
    SendBuffer[5] = (u8)(ID1 >> 16);
    SendBuffer[6] = (u8)(ID2 >> 16);
    SendBuffer[7] = (u8)(ID2 >> 8);
    SendBuffer[8] = (u8)ID1;
    SendBuffer[9] = (u8)(ID0 >> 8);
    SendBuffer[10] = (u8)ID0;
    SendBuffer[11] = (u8)(ID1 >> 8);
    SendBuffer[12] = (u8)ID2;
    SendBuffer[13] = 0;
    SendBuffer[14] = 0;
    SendBuffer[15] = 0;
    SendBuffer[16] = 0;
    SendBuffer[17] = (u8)(ESS_ONE_PLUS_PRODUCT_ID >> 8);
    SendBuffer[18] = (u8)ESS_ONE_PLUS_PRODUCT_ID;

    SendBuffer[19] = (u8)(BOOTLOADER_VERSION >> 24);
    SendBuffer[20] = (u8)(BOOTLOADER_VERSION >> 16);
    SendBuffer[21] = (u8)(BOOTLOADER_VERSION >> 8);
    SendBuffer[22] = (u8)(BOOTLOADER_VERSION);
    for(i = 0; i < 12; i++)
    {
        SendBuffer[23 + i] = HwVersionBuff[i];
    }
    
    UsbSerialSendResponse(SendBuffer, 35);
}


static int MassStorageWriteStatusCmdHandler(unsigned char *buff, unsigned short int cnt)
{
    unsigned char SendBuffer[1];
    if(cnt != 1)
        return 46;

    if(buff[0] >1)
        return 47;

    if(buff[0] == 0)
    {
        FLASH_Lock();
        MassStorageWriteEnableFlag = DISABLE;
    }
    else
    {
        FLASH_Unlock();
        MassStorageWriteEnableFlag = ENABLE;
    }

    SendBuffer[0] = STORAGE_WRITE_STATUS_CMD + 0x80;
    UsbSerialSendResponse(SendBuffer, 1);
    return 0;
}

static void MassStorageReadStatusCmdHandler()
{
    unsigned char SendBuffer[2];


    SendBuffer[0] = STORAGE_READ_STATUS_CMD + 0x80;
    SendBuffer[1] = (u8)MassStorageWriteEnableFlag;

    UsbSerialSendResponse(SendBuffer, 2);
}

static void ReadSystemInfoCmdHandler()
{
    unsigned char SendBuffer[513];
    int i;


    SendBuffer[0] = READ_SYSTEM_INFO + 0x80;
    for(i = 0; i < 512; i++)
    {
        SendBuffer[1 + i] = *(unsigned char *)(SYSTEM_INFO_ADDR + i);
    }

    UsbSerialSendResponse(SendBuffer, 513);
}




static int HwSetCmdHandler(unsigned char *buff, unsigned short int cnt)
{
    unsigned char SendBuffer[1];
    USART_InitTypeDef USART_InitStructure;
    u8 BundrateLevel;
    if(cnt != 10)
        return 60;

    if(buff[0] > 5)
        return 61;

    BundrateLevel = buff[0];

    SendBuffer[0] = HW_SET_CMD + 0x80;
    UsbSerialSendResponse(SendBuffer, 1);

    mDelay(10);


    USART_Cmd(COM_PORT, DISABLE);
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_2;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  

    switch(BundrateLevel)
    {
        case 1:
            USART_InitStructure.USART_BaudRate = 57600;
            break;
        case 2:
            USART_InitStructure.USART_BaudRate = 115200;
            break;
        case 3:
            USART_InitStructure.USART_BaudRate = 230400;
            break;
        case 4:
            USART_InitStructure.USART_BaudRate = 460800;
            break;
        case 5:
            USART_InitStructure.USART_BaudRate = 921600;
            break;
        default:
            USART_InitStructure.USART_BaudRate = 921600;
            break;
    }

    USART_Init(COM_PORT, &USART_InitStructure);
    USART_Cmd(COM_PORT, ENABLE);

    return 0;
}

void UsbSerialDataLayerHandler(unsigned char *buff, unsigned short int cnt)
{
    u16 ret;
    switch(buff[0])
    {
        case WRITE_APPLICATION_DATA_CMD:
            ret = WriteApplicationDataCmdHandler(buff + 1, cnt - 1);
            if(ret != 0)
                UsbSerialSendErrorPacket(ret);
            break;
        case WRITE_APPLICATION_DATA_FINISH_CMD:
	            WriteApplicationDataFinishCmdHandler();
            break;
        case WRITE_APPLICATION_DATA_START_CMD:
            ret = WriteApplicationDataStartCmdHandler(buff + 1, cnt - 1);
            if(ret != 0)
                UsbSerialSendErrorPacket(ret);
            break;
        case RUN_TO_APPLICATION:
            RunToApplicationCmdHandler();
            break;
        case CHECK_WORKING_PROGRAMME_CMD:
            CheckWorkingProgrammeCmdHandler();
            break;
        case READ_SERIAL_NUM_CMD:
            ReadSerialNumCmdHandler();
            break;
        case STORAGE_WRITE_STATUS_CMD:
            ret = MassStorageWriteStatusCmdHandler(buff + 1, cnt - 1);
            if(ret != 0)
                UsbSerialSendErrorPacket(ret);    
            break;
        case STORAGE_READ_STATUS_CMD:
            MassStorageReadStatusCmdHandler();
            break;
        case READ_SYSTEM_INFO:
            ReadSystemInfoCmdHandler();
            break;
        case HW_SET_CMD:
            ret = HwSetCmdHandler(buff + 1, cnt - 1);
            if(ret != 0)
                UsbSerialSendErrorPacket(ret);
            break;
        default:
            UsbSerialSendErrorPacket(2);
            break;
    }
}


