#include "stdint.h"
#include "string.h"
#include "app_util.h"
#include "nrf_error.h"
#include "nrf_dfu_flash.h"
#include "nrf_dfu_utils.h"
#include "nrf_dfu_settings.h"
#include "nrf_bootloader_wdt.h"
#include "nrf_log.h"
#include "FwDecryption.h"


static uint16_t lfsr = 10000;
uint16_t random()
{
	uint16_t bit;
	bit = ((lfsr >> 0) ^ (lfsr >> 2) ^ (lfsr >> 3) ^ (lfsr >> 5)) & 1;
	return lfsr = (lfsr >> 1) | (bit << 15);
}

void srandom(uint16_t seed)
{
	lfsr = seed;
}



void DecryptOnce(unsigned int *v, unsigned int *k) 
 {
     volatile unsigned long y = v[0];
     volatile unsigned long z = v[1];
     unsigned int sum = 0xC6EF3720, i; // set up 
     unsigned int delta = 0x9e3779b9;                  // a key schedule constant 
     unsigned int a = *k;
     unsigned int b = *(unsigned int *)(k + 1);
     unsigned int c = *(unsigned int *)(k + 2);
     unsigned int d = *(unsigned int *)(k + 3);
     for(i = 0; i < 32; i++) {                            // basic cycle start 
         z -= ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);
         y -= ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
         sum -= delta;                                // end cycle 
     }
     v[0] = y;
     v[1] = z;
 }


uint8_t DecryptTempBuff[4096];
/** 
 * [DecryptOneSector description]Offset对一个扇区的数据进行解密，并写入FLASH，数据量应小于等于4096
 * @Author   tin39
 * @DateTime 2019年8月9日T14:34:45+0800
 * @param    DistAddr                 [description]目标FLASH的地址
 * @param    SrcAddr                  [description]加密数据的地址
 * @param    Key                      [description]TEA KEY
 * @param    EncRound                 [description]TEA机密轮数
 * @param    DataCnt                  [description]数据量，不大于4096
 */
uint32_t DecryptAndWriteOneSector(uint32_t DistAddr, uint32_t SrcAddr, uint32_t *Key, uint8_t EncRound, uint32_t DataCnt)
{
    uint32_t ret_val = NRF_SUCCESS;
    uint32_t DecryptCnt;
    uint32_t Offset = 0;
    memcpy(DecryptTempBuff, (uint8_t *)SrcAddr, DataCnt);
    DecryptCnt = DataCnt / 8;
    for(int i = 0; i < DecryptCnt; i++)
    {
        for(int k = 0; k < EncRound; k++)
        {
            DecryptOnce((uint32_t *)&DecryptTempBuff[Offset], Key);
        }
        Offset += 8;
    }

    ret_val = nrf_dfu_flash_erase(DistAddr, 1, NULL);
    if (ret_val != NRF_SUCCESS)
    {
        return ret_val;
    }

    // Flash one page
    //NRF_LOG_DEBUG("Copying 0x%x to 0x%x, size: 0x%x", src_addr, dst_addr, bytes);
    ret_val = nrf_dfu_flash_store(DistAddr,
                                  DecryptTempBuff,
                                  ALIGN_NUM(sizeof(uint32_t), DataCnt),
                                  NULL);
    if (ret_val != NRF_SUCCESS)
    {
        return ret_val;
    }

    return NRF_SUCCESS;
}

uint32_t image_decryption_copy(uint32_t dst_addr,
                           uint32_t src_addr,
                           uint32_t size,
                           uint32_t progress_update_step)
{
    uint32_t ret_val = NRF_SUCCESS;
    uint32_t InfoAddr = nrf_dfu_bank1_start_addr() + s_dfu_settings.bank_1.image_size - SIZE_FW_INFO;
    uint32_t RemainDataCnt;
    uint32_t TeaKey[4];
    uint8_t TeaRound;
    
    if (src_addr == dst_addr)
    {
        //如果是第一次下载，原先APP区域没有内容，则BANK0和BANK1实际FLASH区域重合
        //进行就地解密，擦除，写入
        uint32_t const image_size  = s_dfu_settings.bank_1.image_size - SIZE_FW_INFO;
        uint32_t Addr = nrf_dfu_bank0_start_addr();
        uint32_t SavedDataCnt = 0;
        
        if(image_size % 8 != 0)
        {
            return ERROR_FW_SIZE;
        }

        //每次进行一个扇区的解密及写入
        RemainDataCnt = image_size;
        while(RemainDataCnt > 0)
        {
            uint32_t TeaIndex;
            //每8192字节一个TEA KEY
            if((SavedDataCnt & 0x00001FFF) == 0)//if(SavedDataCnt % 8192 == 0)
            {
                //得到并计算TEA KEY和TEA ROUND
                TeaIndex = SavedDataCnt / 8192;

                uint16_t TeaSeed;
                memcpy(&TeaSeed, (uint8_t *)(InfoAddr + 4 + 3 * TeaIndex), 2);
                TeaRound = ((*(uint8_t *)(InfoAddr + 4 + 3 * TeaIndex + 2)) & 0x07) + 1;//轮数为0-7，太大解密时间会很长
                srandom(TeaSeed);
                for(uint32_t k = 0; k < 4; k++)
                {
                    uint16_t RandomData;
                    RandomData = random();
        			TeaKey[k] = (uint32_t)RandomData << 16;
        			RandomData = random();
        			TeaKey[k] |= RandomData;
                }
            }


            //进行一个扇区的解密及写入
            uint32_t ThisDataCnt;
            if(RemainDataCnt > 4096)
            {
                ThisDataCnt = 4096;
            }
            else
            {
                ThisDataCnt = RemainDataCnt;
            }

            
            ret_val = DecryptAndWriteOneSector(Addr, Addr, TeaKey, TeaRound, ThisDataCnt);
            if(ret_val != NRF_SUCCESS)
            {
                return ret_val;
            }
            
            RemainDataCnt -= ThisDataCnt;
            Addr += 4096;
            SavedDataCnt += ThisDataCnt;
        }

        
        return NRF_SUCCESS;
    }

    ASSERT(src_addr >= dst_addr);
    ASSERT(progress_update_step > 0);
    ASSERT((dst_addr % CODE_PAGE_SIZE) == 0);

    uint32_t max_safe_progress_upd_step = (src_addr - dst_addr)/CODE_PAGE_SIZE;
    ASSERT(max_safe_progress_upd_step > 0);

    
    uint32_t pages_left = CEIL_DIV(size, CODE_PAGE_SIZE);

    //Firmware copying is time consuming operation thus watchdog handling is started
    nrf_bootloader_wdt_init();

    progress_update_step = MIN(progress_update_step, max_safe_progress_upd_step);

    while (size > 0)
    {
        uint32_t pages;
        uint32_t bytes;
        if (pages_left <= progress_update_step)
        {
            pages = pages_left;
            bytes = size;
        }
        else
        {
            pages = progress_update_step;
            bytes = progress_update_step * CODE_PAGE_SIZE;
        }



        uint32_t SrcAddrTmp = src_addr;
        uint32_t DistAddrTmp = dst_addr;

        RemainDataCnt = bytes;

        //这里一次WHILE循环有多个扇区默认是8个，总共需要生成4次TEA KEY
        while(RemainDataCnt > 0)
        {
            uint32_t TeaIndex;
            uint32_t AddrOffset = SrcAddrTmp - nrf_dfu_bank1_start_addr();
            if((AddrOffset & 0x00001FFF) == 0)//if(AddrOffset % 8192 == 0)
            {
                //得到并计算TEA KEY和TEA ROUND
                //每8K区域一个TEA信息
                TeaIndex = AddrOffset / 8192;

                uint16_t TeaSeed;
                memcpy(&TeaSeed, (uint8_t *)(InfoAddr + 4 + 3 * TeaIndex), 2);
                TeaRound = ((*(uint8_t *)(InfoAddr + 4 + 3 * TeaIndex + 2)) & 0x07) + 1;//轮数为0-7，太大解密时间会很长
                srandom(TeaSeed);
                for(uint32_t k = 0; k < 4; k++)
                {
                    uint16_t RandomData;
                    RandomData = random();
        			TeaKey[k] = (uint32_t)RandomData << 16;
        			RandomData = random();
        			TeaKey[k] |= RandomData;
                }
            }

            //进行一个扇区的解密及写入
            uint32_t ThisDataCnt;
            if(RemainDataCnt > 4096)
            {
                ThisDataCnt = 4096;
            }
            else
            {
                ThisDataCnt = RemainDataCnt;
            }

            
            ret_val = DecryptAndWriteOneSector(DistAddrTmp, SrcAddrTmp, TeaKey, TeaRound, ThisDataCnt);
            if(ret_val != NRF_SUCCESS)
            {
                return ret_val;
            }

            SrcAddrTmp += 4096;
            DistAddrTmp += 4096;
            RemainDataCnt -= ThisDataCnt;
    
        }

        pages_left  -= pages;
        size        -= bytes;
        dst_addr    += bytes;
        src_addr    += bytes;
        s_dfu_settings.write_offset += bytes;

        //store progress in flash on every successful chunk write
        ret_val = nrf_dfu_settings_write_and_backup(NULL);
        if (ret_val != NRF_SUCCESS)
        {
            NRF_LOG_ERROR("Failed to write image copying progress to settings page.");
            return ret_val;
        }
    }

    return ret_val;
}

