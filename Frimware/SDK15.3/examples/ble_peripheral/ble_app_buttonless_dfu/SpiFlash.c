#include "common.h"
#include <stdint.h>
#include "nrf_spim.h"
#include "nrf_gpio.h"
#include "SpiFlash.h"
//#include "Error.h"

#define MAX_CNT_SPI_OPERATE     0xFF

#define FLASH_SPI			    NRF_SPIM0
#define PIN_FLASH_SPI_SCK       19
#define PIN_FLASH_SPI_MOSI      20
#define PIN_FLASH_SPI_MISO      18
#define PIN_FLASH_SPI_CS        17

//#define PIN_SPI_TEST            25

//#define TEST_PIN_LOW            //NRF_P0->OUTCLR = 1UL << PIN_SPI_TEST
//#define TEST_PIN_HIGH           //NRF_P0->OUTSET = 1UL << PIN_SPI_TEST

#define SPI_FLASH_CS_LOW()       NRF_P0->OUTCLR = 1UL << PIN_FLASH_SPI_CS//;nrf_gpio_pin_clear(PIN_FLASH_SPI_CS)
#define SPI_FLASH_CS_HIGH()      NRF_P0->OUTSET = 1UL << PIN_FLASH_SPI_CS//nrf_gpio_pin_set(PIN_FLASH_SPI_CS)


#define WINBOND_8M_SSID					0xEF4017
#define WINBOND_8M_FLASH_BLOCK_CNT		128

#define WINBOND_4M_SSID					0xEF4016
#define WINBOND_4M_FLASH_BLOCK_CNT		64

#define GD_16M_SSID							0xC84018
#define GD_16M_FLASH_BLOCK_CNT				256

#define GD_8M_SSID							0xC84017
#define GD_8M_FLASH_BLOCK_CNT				128

#define GD_4M_SSID							0xC84016
#define GD_4M_FLASH_BLOCK_CNT				64

#define GD_2M_SSID							0xC84015
#define GD_2M_FLASH_BLOCK_CNT				32

#define GD_1M_SSID							0xC84014
#define GD_1M_FLASH_BLOCK_CNT				16



void SoftwareDisableWriteProtoct(void);
uint32_t SstReadId(void);



struct SpiFlashInfo SpiFlash;

void SpiFlashHwInit(void)
{
    //nrf_gpio_cfg_output(PIN_SPI_TEST);
    
    nrf_gpio_cfg_output(PIN_FLASH_SPI_CS);
    SPI_FLASH_CS_HIGH();					  

    nrf_spim_shorts_disable(FLASH_SPI, NRF_SPIM_SHORT_END_START_MASK);
    nrf_spim_int_disable(FLASH_SPI, NRF_SPIM_INT_STOPPED_MASK | NRF_SPIM_INT_ENDRX_MASK    | NRF_SPIM_INT_END_MASK |
                            NRF_SPIM_INT_ENDTX_MASK | NRF_SPIM_INT_STARTED_MASK);
    nrf_spim_pins_set(FLASH_SPI, PIN_FLASH_SPI_SCK, PIN_FLASH_SPI_MOSI, PIN_FLASH_SPI_MISO);
    nrf_spim_frequency_set(FLASH_SPI, NRF_SPIM_FREQ_8M);
    nrf_spim_configure(FLASH_SPI, NRF_SPIM_MODE_0, NRF_SPIM_BIT_ORDER_MSB_FIRST);    
    nrf_spim_orc_set(FLASH_SPI, 0xFF);
    nrf_spim_tx_list_disable(FLASH_SPI);
    nrf_spim_rx_list_disable(FLASH_SPI);
    

    nrf_spim_enable(FLASH_SPI);
   

    

   
  
    SpiFlash.Ssid = SstReadId();
    if(SpiFlash.Ssid == WINBOND_8M_SSID)
    {
        SpiFlash.FlashBlockCnt = WINBOND_8M_FLASH_BLOCK_CNT;
        SpiFlash.FlashStatus = ENABLE;
        
    }
    else if(SpiFlash.Ssid == WINBOND_4M_SSID)
    {
        SpiFlash.FlashBlockCnt = WINBOND_4M_FLASH_BLOCK_CNT;
        SpiFlash.FlashStatus = ENABLE;
    }
    else if(SpiFlash.Ssid == GD_16M_SSID)
    {
        SpiFlash.FlashBlockCnt = GD_16M_FLASH_BLOCK_CNT;
        SpiFlash.FlashStatus = ENABLE;
    }
    else if(SpiFlash.Ssid == GD_8M_SSID)
    {
        SpiFlash.FlashBlockCnt = GD_8M_FLASH_BLOCK_CNT;
        SpiFlash.FlashStatus = ENABLE;
    }
    else if(SpiFlash.Ssid == GD_4M_SSID)
    {
        SpiFlash.FlashBlockCnt = GD_4M_FLASH_BLOCK_CNT;
        SpiFlash.FlashStatus = ENABLE;
    }
    else if(SpiFlash.Ssid == GD_2M_SSID)
    {
        SpiFlash.FlashBlockCnt = GD_2M_FLASH_BLOCK_CNT;
        SpiFlash.FlashStatus = ENABLE;
    }
    else if(SpiFlash.Ssid == GD_1M_SSID)
    {
        SpiFlash.FlashBlockCnt = GD_1M_FLASH_BLOCK_CNT;
        SpiFlash.FlashStatus = ENABLE;
    }
    else
    {
        //ReportError(ERROR_FLASH_INIT_ERROR);
        SpiFlash.FlashStatus = DISABLE;
    }
    SpiFlash.FlashBlockSize = SPI_FLASH_BLOCK_SIZE;
    SpiFlash.FlashSize = SpiFlash.FlashBlockCnt * SpiFlash.FlashBlockSize;
  
    SoftwareDisableWriteProtoct();
}

int32_t CheckEmpty(uint8_t *buff, int32_t cnt)
{
    int32_t i;
    for(i = 0; i < cnt; i++)
    {
        if(buff[i] != 0xFF)
        {
            return 1;
        }
    }
    return 0;
}


void SpiReadWrite(uint8_t *TxBuff, uint8_t *RxBuff, uint32_t DataCnt)
{    
    uint32_t RdWrCnt = 0;
    while(RdWrCnt < DataCnt)
    {
        uint32_t ThisRdWrCnt;
        uint32_t RemainCnt = DataCnt - RdWrCnt;
        if(RemainCnt > MAX_CNT_SPI_OPERATE)
        {
            ThisRdWrCnt = MAX_CNT_SPI_OPERATE;
        }
        else
        {
            ThisRdWrCnt = RemainCnt;
        }

        nrf_spim_tx_buffer_set(FLASH_SPI, TxBuff + RdWrCnt, ThisRdWrCnt);
        nrf_spim_rx_buffer_set(FLASH_SPI, RxBuff + RdWrCnt, ThisRdWrCnt);
        //nrf_spim_event_clear(FLASH_SPI, NRF_SPIM_EVENT_END);
        *((volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_EVENT_END)) = 0x0UL;
        
        //nrf_spim_task_trigger(FLASH_SPI, NRF_SPIM_TASK_START);
        *((volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_TASK_START)) = 0x1UL;
        
        //while(!nrf_spim_event_check(FLASH_SPI, NRF_SPIM_EVENT_END));
        while(!(bool)*(volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_EVENT_END));
        
        //nrf_spim_event_clear(FLASH_SPI, NRF_SPIM_EVENT_END);
        *((volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_EVENT_END)) = 0x0UL;

        RdWrCnt += ThisRdWrCnt;
    }
    
    
}

void SpiWrite(uint8_t *TxBuff, uint32_t DataCnt)
{    
    uint32_t WrittenDataCnt = 0;

    while(WrittenDataCnt < DataCnt)
    {
        uint32_t ThisWriteCnt;
        uint32_t RemainCnt = DataCnt - WrittenDataCnt;
        if(RemainCnt > MAX_CNT_SPI_OPERATE)
        {
            ThisWriteCnt = MAX_CNT_SPI_OPERATE;
        }
        else
        {
            ThisWriteCnt = RemainCnt;
        }
        nrf_spim_tx_buffer_set(FLASH_SPI, TxBuff + WrittenDataCnt, ThisWriteCnt);    
        nrf_spim_rx_buffer_set(FLASH_SPI, NULL, 0);

        //nrf_spim_event_clear(FLASH_SPI, NRF_SPIM_EVENT_ENDTX);
        *((volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_EVENT_ENDTX)) = 0x0UL;
        //nrf_spim_task_trigger(FLASH_SPI, NRF_SPIM_TASK_START);
        *((volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_TASK_START)) = 0x1UL;
        
        //while(!nrf_spim_event_check(FLASH_SPI, NRF_SPIM_EVENT_ENDTX));
        while(!(bool)*(volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_EVENT_ENDTX));
        //nrf_spim_event_clear(FLASH_SPI, NRF_SPIM_EVENT_ENDTX);
        *((volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_EVENT_ENDTX)) = 0x0UL;

        WrittenDataCnt += ThisWriteCnt;
    }
}
    

void SpiRead(uint8_t *RxBuff, uint32_t DataCnt)
{    
    uint32_t ReadDataCnt = 0;
    while(ReadDataCnt < DataCnt)
    {
        uint32_t ThisReadCnt;
        uint32_t RemainCnt = DataCnt - ReadDataCnt;
        if(RemainCnt > MAX_CNT_SPI_OPERATE)
        {
            ThisReadCnt = MAX_CNT_SPI_OPERATE;
        }
        else
        {
            ThisReadCnt = RemainCnt;
        }
        nrf_spim_tx_buffer_set(FLASH_SPI, NULL, 0);    
        nrf_spim_rx_buffer_set(FLASH_SPI, RxBuff + ReadDataCnt, ThisReadCnt);
    
        //nrf_spim_event_clear(FLASH_SPI, NRF_SPIM_EVENT_ENDRX);
        *((volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_EVENT_ENDRX)) = 0x0UL;
        //nrf_spim_task_trigger(FLASH_SPI, NRF_SPIM_TASK_START);
        *((volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_TASK_START)) = 0x1UL;
        
        //while(!nrf_spim_event_check(FLASH_SPI, NRF_SPIM_EVENT_ENDRX));
        while(!(bool)*(volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_EVENT_ENDRX));
        //nrf_spim_event_clear(FLASH_SPI, NRF_SPIM_EVENT_ENDRX);
        *((volatile uint32_t *)((uint8_t *)FLASH_SPI + (uint32_t)NRF_SPIM_EVENT_ENDRX)) = 0x0UL;

        ReadDataCnt += ThisReadCnt;
    }
}



void WriteEnable()
{
    uint8_t Data = 0x06;
    SPI_FLASH_CS_LOW();  
    SpiWrite(&Data, 1);
    SPI_FLASH_CS_HIGH();
}


void WaitFlashFree()
{
    uint8_t TxBuff[2] = {0x05, 0x00};
    uint8_t RxBuff[2];


    SPI_FLASH_CS_LOW();  
    do
    {
   	    SpiReadWrite(TxBuff, RxBuff, 2);
    }while(RxBuff[1] & 0x01);
    SPI_FLASH_CS_HIGH();
}

void SoftwareDisableWriteProtoct()
{
    uint8_t TxBuff[3] = {0x01, 0x80, 0x00};
    WaitFlashFree();
    WriteEnable();
    
    SPI_FLASH_CS_LOW();  
    SpiWrite(TxBuff, 3);
    SPI_FLASH_CS_HIGH();
}

uint32_t SstReadId()
{
    uint8_t RxBuff[4];
    uint8_t TxBuff[4] = {0x9F, 0x00, 0x00, 0x00};
    uint32_t SstId;
    WaitFlashFree();	
    SPI_FLASH_CS_LOW();
    SpiReadWrite(TxBuff, RxBuff, 4);
    SPI_FLASH_CS_HIGH();

    SstId = ((RxBuff[1] << 16) | (RxBuff[2] << 8) | RxBuff[3]);
    return SstId;
}

void SectorErase(unsigned int Addr)
{
    uint8_t TxBuff[4];
    TxBuff[0] = 0x20;
    TxBuff[1] = (uint8_t)(Addr >> 16);	
    TxBuff[2] = (uint8_t)((Addr >> 8) & 0xF0);
    TxBuff[3] = 0x00;
    
    WaitFlashFree();
    WriteEnable();

    SPI_FLASH_CS_LOW();
    SpiWrite(TxBuff, 4);
    SPI_FLASH_CS_HIGH();
    WaitFlashFree();
}

void BlockErase(unsigned int Addr)
{
    uint8_t TxBuff[4];
    TxBuff[0] = 0xD8;
    TxBuff[1] = (unsigned char)(Addr >> 16);				
    TxBuff[2] = 0;
    TxBuff[3] = 0x00;
        

    WaitFlashFree();
    WriteEnable();
    	
    SPI_FLASH_CS_LOW();
    SpiWrite(TxBuff, 4);
    SPI_FLASH_CS_HIGH();

    WaitFlashFree();
}

void ChipErase()
{
    uint8_t data = 0x60;
    WaitFlashFree();
    WriteEnable();
   		
	
    SPI_FLASH_CS_LOW();
    SpiWrite(&data, 1);  
    SPI_FLASH_CS_HIGH();

    WaitFlashFree();
}

uint32_t DataFlashReadData(uint32_t StartAddress, uint8_t *ucRdDataBuff, uint32_t ReadCnt)
{
    uint8_t Cmd[4];
    
    Cmd[0] = 0x03;
    Cmd[1] = (StartAddress >> 16) & 0xFF;
    Cmd[2] = (StartAddress >> 8) & 0xFF;
    Cmd[3] = StartAddress & 0xFF;
    
    WaitFlashFree();
    
    SPI_FLASH_CS_LOW();

    SpiWrite(Cmd, 4);
    SpiRead(ucRdDataBuff, ReadCnt);
    
    SPI_FLASH_CS_HIGH();
    return 0;
}

void DataFlashDirectWriteData(uint32_t StartAddress, uint8_t *ucWrDataBuff, uint32_t WriteCnt)
{
    uint32_t ThisWriteCnt;
    uint8_t *WritePtr;

    WritePtr = ucWrDataBuff;
    while(WriteCnt > 0)
    {
        if((StartAddress + WriteCnt) > ((StartAddress & 0xFFFFFF00) + SPI_FLASH_PAGE_SIZE))
        {
            ThisWriteCnt = ((StartAddress & 0xFFFFFF00) + SPI_FLASH_PAGE_SIZE) - StartAddress;
        }
        else
        {
            ThisWriteCnt = WriteCnt;
        }
        
        WaitFlashFree();
        WriteEnable();
        SPI_FLASH_CS_LOW();

        uint8_t TxBuff[4];
        TxBuff[0] = 0x02;
        TxBuff[1] = (StartAddress >> 16) & 0xFF;
        TxBuff[2] = (StartAddress >> 8) & 0xFF;
        TxBuff[3] = StartAddress & 0xFF;  
        SpiWrite(TxBuff, 4);
        SpiWrite(WritePtr, ThisWriteCnt);
 	             
        SPI_FLASH_CS_HIGH();

        WritePtr += ThisWriteCnt;
        WriteCnt -= ThisWriteCnt;
        StartAddress += ThisWriteCnt;
    }
    WaitFlashFree();
}

int FlashWriteCheck(uint32_t StartAddress, uint32_t DataCnt, uint8_t *WittenData, uint8_t *FlashDataBuff)
{
    uint32_t i;
    DataFlashReadData(StartAddress, FlashDataBuff, DataCnt);
    for(i = 0; i < DataCnt; i++)
    {
        if(*WittenData != *FlashDataBuff)
        {
            return -1;
        }
        WittenData++;
        FlashDataBuff++;
    }
    return DataCnt;
}



//返回剩余要写入数据的数量
uint32_t DataFlashWriteData(uint32_t Addr, uint8_t *ucWrDataBuff, uint32_t DataCnt)
{
    int32_t i;
    int32_t BuffOffset;
    uint8_t *WritePtr;
    uint8_t ReadTemp[SPI_FLASH_SECTOR_SIZE];
    int32_t ThisWriteCnt;
    uint32_t TotalDataCnt = DataCnt;
    uint32_t OriAddr = Addr;
    int CheckRet;
    WritePtr = ucWrDataBuff;
    while(DataCnt)
    {
        //一个扇区范围内的处理
        BuffOffset = Addr & 0xFFF;
        if(BuffOffset + DataCnt <= SPI_FLASH_SECTOR_SIZE)
        {
            //写入的数据在一个扇区范围内
            ThisWriteCnt = DataCnt;
        }
        else
        {
            //写入的数据超出一个扇区范围外
            ThisWriteCnt = SPI_FLASH_SECTOR_SIZE - BuffOffset;
        }

        DataFlashReadData(Addr, ReadTemp, ThisWriteCnt);
        if(CheckEmpty(ReadTemp, ThisWriteCnt))
        {
            //写入的区域不为空，将整个扇区读取出来，再将需要写入的数据填充到缓冲中，再整个扇区写入
            DataFlashReadData((Addr & 0xFFFFF000), ReadTemp, SPI_FLASH_SECTOR_SIZE);
            SectorErase(Addr);
            for(i = 0; i < ThisWriteCnt; i++)
            {
                ReadTemp[BuffOffset + i] = WritePtr[i];
            }
            DataFlashDirectWriteData((Addr & 0xFFFFF000),  ReadTemp, SPI_FLASH_SECTOR_SIZE);
        }
        else
        {
            //写入的区域为空，直接进行写入操作
            DataFlashDirectWriteData(Addr, WritePtr, ThisWriteCnt);
        }
        WritePtr += ThisWriteCnt;
        Addr += ThisWriteCnt;
        DataCnt -= ThisWriteCnt;
    }

    CheckRet = FlashWriteCheck(OriAddr, TotalDataCnt, ucWrDataBuff, ReadTemp);

    return CheckRet;
}

//uint8_t ReadBuff[10240];
//uint8_t WriteBuff[10240];
//uint32_t ReadFlag;
//uint32_t WriteFlag;
//uint32_t EraseFlag;
//uint32_t Addr;
//uint32_t DataCnt = 4096;
//void FlashTest()
//{
//    int i, j;
//    for(i = 0; i < 40; i++)
//    {
//        for(j = 0; j < 256; j++)
//        {
//            WriteBuff[i * 256 + j] = (uint8_t)i;
//        }
//        
//    }
//    while(1)
//    {
//        if(ReadFlag != 0)
//        {
//            ReadFlag = 0;
//            for(i = 0; i < 4096; i++)
//            {
//                ReadBuff[i] = 0;
//            }
//            DataFlashReadData(Addr, ReadBuff, DataCnt);
//        }
//    
//        if(WriteFlag != 0)
//        {
//            WriteFlag = 0;
//            DataFlashWriteData(Addr, WriteBuff, DataCnt);
//        }
//    
//        if(EraseFlag != 0)
//        {
//            EraseFlag = 0;
//            SectorErase(Addr);
//        }
//    }
//    
//    
//}


