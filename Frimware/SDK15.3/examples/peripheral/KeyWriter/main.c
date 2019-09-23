/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 * @defgroup flashwrite_example_main main.c
 * @{
 * @ingroup flashwrite_example
 *
 * @brief This file contains the source code for a sample application using the Flash Write Application.
 *a
 */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "nrf.h"
#include "bsp.h"
#include "app_error.h"
#include "nrf_nvmc.h"
#include "nordic_common.h"


#include "app_timer.h"
#include "nrf_drv_clock.h"

#define OFFSET_MODULUS										1
#define OFFSET_PUBLIC_EXPONENT								68
#define OFFSET_EXPONENT										168
#define OFFSET_PRIME0										250			
#define OFFSET_PRIME1										301
#define OFFSET_PRIME_EXPONENT0								364
#define OFFSET_PRIME_EXPONENT1								402
#define OFFSET_COEFFICIENT									479


#define OFFSET_FW_MODULUS										700
#define OFFSET_FW_PUBLIC_EXPONENT								780
#define OFFSET_FW_EXPONENT										1021
#define OFFSET_FW_PRIME0										1341			
#define OFFSET_FW_PRIME1										1451
#define OFFSET_FW_PRIME_EXPONENT0								1521
#define OFFSET_FW_PRIME_EXPONENT1								1600
#define OFFSET_FW_COEFFICIENT									1690


#define OFFSET_SECRET												580




//#define  SECRET_ADDRESS		0x00061000
#define FLASH_SECTOR_SIZE     4096
unsigned char happy[] = {"Ess Ev"};//29
unsigned char free[] = {"Awesome!"};//31
uint32_t FlashAddr;

uint8_t Buff[FLASH_SECTOR_SIZE];
uint8_t FlashContent[FLASH_SECTOR_SIZE];



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


void SetBuffRandom()
{
    uint32_t i;
    uint16_t RandomData;
    srandom(0x55AA);
    for(i = 0; i < FLASH_SECTOR_SIZE / 2; i++)
    {
        RandomData = random();
        memcpy(Buff + i * 2, &RandomData, 2);
    }
}

static void flash_page_erase(uint32_t * page_address)
{
    // Turn on flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Een << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Erase page:
    NRF_NVMC->ERASEPAGE = (uint32_t)page_address;

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash erase enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
}


/** @brief Function for filling a page in flash with a value.
 *
 * @param[in] address Address of the first word in the page to be filled.
 * @param[in] value Value to be written to flash.
 */
static void flash_word_write(uint32_t * address, uint32_t value)
{
    // Turn on flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Wen << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    *address = value;

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }

    // Turn off flash write enable and wait until the NVMC is ready:
    NRF_NVMC->CONFIG = (NVMC_CONFIG_WEN_Ren << NVMC_CONFIG_WEN_Pos);

    while (NRF_NVMC->READY == NVMC_READY_READY_Busy)
    {
        // Do nothing.
    }
}



void MixerBuffWithID(uint8_t *buff)
{
    uint32_t Id[2];
    int32_t i;
    Id[0] = NRF_FICR->DEVICEID[0];
    Id[1] = NRF_FICR->DEVICEID[1];

    for(i = 0; i < FLASH_SECTOR_SIZE / 32; i++)
    {
        uint32_t temp[8];
        for(int j = 0; j < 8; j++)
        {
            memcpy(&temp[j], (uint8_t *)(buff + 32 * i + j * 4), 4);
        }

        temp[0] = temp[0] ^ Id[0];
        temp[1] = temp[1] ^ Id[1];
        temp[2] = temp[2] + Id[1];
        temp[3] = temp[3] - Id[0];
        temp[4] = temp[4] + Id[0] + 0x55555555;
        temp[5] = temp[5] - Id[1] - 0x12563214;
        temp[6] = temp[6] ^ Id[1] ^ 0x5412365F;
        temp[7] = temp[7] ^ Id[0] ^ 0xDEF1A9B0;

        for(int j = 0; j < 8; j++)
        {
            memcpy((uint8_t *)(buff + 32 * i + j * 4), &temp[j], 4);
        }
    }
    
}





unsigned char modulus[64] = {0xC4, 0xD9, 0xF2, 0x02, 0xF2, 0xFB, 0xAB, 0x93, 0x2D, 0x09, 0xDF, 0xC4, 0xC0, 0xBB, 0x10, 0xB1, 0xFD, 0x04, 0x8A, 0xA7, 0xCC, 0xDF, 0x2D, 0x51, 0xED, 0x4A, 0x8B, 0xB0, 0xD0, 0x9C, 0x7A, 0x67, 0x69, 0x2B, 0x5F, 0xE8, 0x0A, 0xAD, 0xA9, 0x1C, 0x39, 0x05, 0x69, 0x9C, 0xCA, 0x77, 0xC4, 0x55, 0xD7, 0x4D, 0x58, 0x70, 0xEC, 0x80, 0xD9, 0x78, 0x99, 0xB1, 0xC3, 0x00, 0x5A, 0x59, 0x1C, 0xBD};
unsigned char publicExponent[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01}; 
unsigned char exponent[64] = {0x45, 0x3D, 0x94, 0x0E, 0xCD, 0x81, 0x10, 0x9C, 0xB8, 0x1E, 0xDD, 0x02, 0xCB, 0xF8, 0x00, 0xAB, 0x5A, 0xA0, 0x1A, 0x55, 0x63, 0x8E, 0xAC, 0xD6, 0x40, 0x6D, 0x14, 0x2D, 0xA1, 0x73, 0x28, 0xEA, 0x50, 0xD7, 0x52, 0x40, 0x18, 0x4D, 0x4B, 0xE8, 0x25, 0x73, 0xC8, 0xAD, 0x3F, 0xA0, 0x66, 0xB2, 0x9F, 0x3E, 0x93, 0xB5, 0x95, 0x0A, 0xB4, 0xFE, 0xA7, 0x12, 0xE7, 0x61, 0xFA, 0xEE, 0x2F, 0x81};
unsigned char prime1[32] = {0xEA, 0x2A, 0x2B, 0x97, 0xC3, 0x54, 0x2B, 0x36, 0x1B, 0x23, 0x38, 0x9A, 0x90, 0x06, 0x85, 0xD5, 0x60, 0x27, 0xE9, 0xCC, 0xF6, 0x34, 0x06, 0x04, 0x5A, 0x76, 0x93, 0x83, 0xBB, 0x7D, 0xA7, 0x95};
unsigned char prime2[32] = {0xD7, 0x35, 0x0D, 0xD9, 0xE9, 0x78, 0xCE, 0x76, 0x8F, 0x4A, 0xBC, 0x4D, 0x6E, 0x21, 0x64, 0xAB, 0xAA, 0x70, 0x84, 0x0C, 0xBE, 0x5E, 0xD6, 0xA3, 0x27, 0xB7, 0x0C, 0x9B, 0x33, 0x7D, 0x36, 0x89};
unsigned char primeExponent1[32] = {0x20, 0x34, 0x1E, 0x7C, 0xA7, 0xA3, 0x4A, 0xB2, 0x0D, 0x37, 0x61, 0xD0, 0x77, 0xE0, 0x3D, 0xC5, 0xA3, 0x8C, 0xB9, 0xAD, 0xB3, 0x6A, 0x62, 0x2D, 0x75, 0x17, 0x7B, 0xA0, 0x11, 0x47, 0xED, 0xA5}; 
unsigned char primeExponent2[32] = {0x67, 0x55, 0x46, 0x2D, 0x57, 0xF9, 0x75, 0xC9, 0x5F, 0xCC, 0x56, 0xD6, 0x27, 0x07, 0x49, 0xBB, 0x53, 0xBF, 0x6A, 0xE8, 0x63, 0x90, 0x4E, 0x27, 0x51, 0x2E, 0x87, 0x93, 0x0E, 0x0A, 0x15, 0x61};
unsigned char coefficient[32] = {0x7D, 0x27, 0xEA, 0xFC, 0x8B, 0xFD, 0xBB, 0xA9, 0x14, 0x72, 0x41, 0x96, 0x79, 0xF0, 0xB8, 0xF2, 0xB9, 0xFF, 0x3C, 0xC1, 0xFA, 0x1A, 0x54, 0x33, 0x7C, 0xA0, 0x3B, 0x06, 0x68, 0x91, 0xB6, 0x95};   

unsigned char FWmodulus[64] = {0xBA, 0x4C, 0x20, 0xAE, 0xA3, 0x89, 0xA1, 0x93, 0x2C, 0x4C, 0xEF, 0xB2, 0x12, 0xAC, 0x25, 0xD7, 0x35, 0x2A, 0x55, 0x8C, 0x24, 0xCA, 0x65, 0x37, 0x5E, 0x56, 0xCA, 0xCC, 0x15, 0xE9, 0x71, 0x75, 0xB7, 0x77, 0x2A, 0x6C, 0x0F, 0xBC, 0xDA, 0x6C, 0xA1, 0x99, 0xB8, 0x0F, 0xF8, 0x13, 0x82, 0xD3, 0x55, 0xC9, 0x9F, 0x6D, 0x41, 0xFD, 0x1A, 0x01, 0x09, 0x12, 0x13, 0xCF, 0xCD, 0x31, 0x4B, 0x25};
unsigned char FWpublicExponent[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01}; 
unsigned char FWexponent[64] = {0x21, 0x5F, 0x59, 0xCB, 0x6F, 0x08, 0x4C, 0xA5, 0x6C, 0x9E, 0x2B, 0xA2, 0x1E, 0xAE, 0x2F, 0xE9, 0x29, 0x60, 0xF8, 0x0A, 0x2E, 0xDD, 0x02, 0xDC, 0xB4, 0x4D, 0x9F, 0xEE, 0x87, 0x4E, 0x56, 0x40, 0x8D, 0xB4, 0x60, 0x13, 0x09, 0x85, 0x25, 0x58, 0xA7, 0x26, 0x2B, 0x64, 0xC5, 0x9B, 0x5F, 0xFC, 0xF3, 0x48, 0x6A, 0x3B, 0x55, 0xA5, 0x7F, 0xEF, 0xAA, 0x58, 0xE8, 0x04, 0xBC, 0xFE, 0x68, 0x01};
unsigned char FWprime1[32] = {0xEB, 0x20, 0x35, 0x3B, 0x41, 0xD1, 0xB2, 0x44, 0xB8, 0x1F, 0x9E, 0xAD, 0xFB, 0x1E, 0x00, 0x92, 0xE1, 0xB1, 0x0F, 0xB5, 0x32, 0xC0, 0xB1, 0xC2, 0xD7, 0x4C, 0xB6, 0xF4, 0x47, 0x29, 0x01, 0x81};
unsigned char FWprime2[32] = {0xCA, 0xD6, 0x2D, 0x76, 0x5F, 0xAA, 0x1C, 0xF3, 0xBD, 0xE4, 0x1B, 0x69, 0x77, 0xAD, 0xC6, 0xCD, 0x55, 0xB6, 0x66, 0xFD, 0xC0, 0xE4, 0x30, 0xA8, 0x08, 0x96, 0xD2, 0x39, 0x5A, 0x86, 0xD3, 0xA5};
unsigned char FWprimeExponent1[32] = {0xDF, 0x80, 0x71, 0x39, 0x4F, 0xDA, 0x9E, 0x43, 0x96, 0x66, 0x70, 0x36, 0xDF, 0xA2, 0xE8, 0x56, 0xB6, 0xF4, 0xD1, 0x82, 0xA3, 0xA8, 0xEA, 0x10, 0xD5, 0x09, 0xD6, 0x4B, 0xA4, 0x2D, 0x05, 0x81}; 
unsigned char FWprimeExponent2[32] = {0x95, 0x02, 0x2C, 0x99, 0xC6, 0x83, 0x81, 0x0B, 0x6A, 0x97, 0xB5, 0xAF, 0x90, 0x35, 0x00, 0x53, 0xFE, 0x40, 0xA6, 0x7E, 0x8A, 0x02, 0xD8, 0xDF, 0xC8, 0x97, 0x8B, 0xCE, 0x88, 0x7B, 0x7D, 0xE5};
unsigned char FWcoefficient[32] = {0x1E, 0xD4, 0xBD, 0x79, 0x02, 0x8C, 0x23, 0xC1, 0x5E, 0x64, 0x12, 0x80, 0x01, 0xC6, 0xCA, 0xCD, 0x4B, 0x81, 0x51, 0x3D, 0x93, 0x45, 0xD2, 0x58, 0x21, 0x68, 0x00, 0xD1, 0x31, 0x91, 0x8D, 0x85};   


unsigned char PublicKey[64] = {0xB9, 0x88, 0x05, 0x50, 0x07, 0x6C, 0x5A, 0x74, 0x07, 0xB9, 0x60, 0x57, 0x9A, 0xB4, 0x0F, 0xE0, 0xED, 0xAA, 0xE4, 0x62, 0xD1, 0xA3, 0x02, 0xCE, 0xAD, 0x96, 0xCD, 0x57, 0xCB, 0x78, 0xD9, 0x18, 0xF2, 0xC1, 0x57, 0x3C, 0xFB, 0x56, 0xFB, 0xCD, 0x90, 0xFC, 0x24, 0x3D, 0xA7, 0xB5, 0xE8, 0x14, 0xC8, 0x16, 0xF9, 0xC3, 0x5E, 0x06, 0xC9, 0x2A, 0xD0, 0xD4, 0x70, 0xCE, 0x0F, 0xF4, 0xFC, 0xCD};


//void GetRadomBuff(unsigned char *buff, int cnt)
//{
//    volatile uint32_t ID1;
//    int i;
//    ID1 = *(volatile uint32_t*)(ID1_ADDR);
//    for(i = 0; i < cnt / 4; i++)
//    {
//        buff[i * 4] = buff[i * 4] ^ (ID1 >> 24);
//        buff[i * 4 + 1] = buff[i * 4 + 1] ^ (ID1 >> 16);
//        buff[i * 4 + 2] = buff[i * 4 + 2] ^ (ID1 >> 8);
//        buff[i * 4 + 3] = buff[i * 4 + 3] ^ ID1;
//    }
//}


//RSA密钥存放的空间在1K以内
void MixRsaBuff(unsigned char *buff, int cnt, int StartOffset)
{
    volatile uint32_t ID2;
    int i;
    ID2 = NRF_FICR->DEVICEID[1];

    for(i = 0; i < cnt / 4; i++)
    {
        buff[i * 4] = buff[i * 4] ^ (ID2 >> 24);
        buff[i * 4 + 1] = buff[i * 4 + 1] ^ (ID2 >> 16);
        buff[i * 4 + 2] = buff[i * 4 + 2] ^ (ID2 >> 8);
        buff[i * 4 + 3] = buff[i * 4 + 3] ^ ID2;
    }

    for(i = 0; i < cnt; i++)
    {
        Buff[StartOffset + i] = buff[i];
    }

}



void MixRsaKey()
{
    MixRsaBuff(modulus, 64, OFFSET_MODULUS);
    MixRsaBuff(publicExponent, 64, OFFSET_PUBLIC_EXPONENT);
    MixRsaBuff(exponent, 64, OFFSET_EXPONENT);
    MixRsaBuff(prime1, 32, OFFSET_PRIME0);
    MixRsaBuff(prime2, 32, OFFSET_PRIME1);
    MixRsaBuff(primeExponent1, 32, OFFSET_PRIME_EXPONENT0);
    MixRsaBuff(primeExponent2, 32, OFFSET_PRIME_EXPONENT1);
    MixRsaBuff(coefficient, 32, OFFSET_COEFFICIENT);
}

void MixFwRsaKey()
{
    MixRsaBuff(FWmodulus, 64, OFFSET_FW_MODULUS);
    MixRsaBuff(FWpublicExponent, 64, OFFSET_FW_PUBLIC_EXPONENT);
    MixRsaBuff(FWexponent, 64, OFFSET_FW_EXPONENT);
    MixRsaBuff(FWprime1, 32, OFFSET_FW_PRIME0);
    MixRsaBuff(FWprime2, 32, OFFSET_FW_PRIME1);
    MixRsaBuff(FWprimeExponent1, 32, OFFSET_FW_PRIME_EXPONENT0);
    MixRsaBuff(FWprimeExponent2, 32, OFFSET_FW_PRIME_EXPONENT1);
    MixRsaBuff(FWcoefficient, 32, OFFSET_FW_COEFFICIENT);
}

void MixAnotherPublicKey()
{
    MixRsaBuff(PublicKey, 64, 888);
    //此处的publicExponent同私钥的publicExponent，故而不写入
}


void MixChipId()
{
    unsigned char MyWish[] = {"I hope ererything goes well."};//28
    volatile uint32_t ID0, ID2, ID1;
    volatile uint8_t *ptr;
	volatile uint32_t *PtrTemp;
    volatile uint32_t i, j;
	volatile uint32_t DataTemp;
    uint32_t temp;
    unsigned char FlashId[12] = {"just  change"};
    ID0 = NRF_FICR->DEVICEID[0];
    ID1 = NRF_FICR->DEVICEID[1];
    ID2 = ID0 ^ ID1;

    
    ptr = (uint8_t *)happy;
    for(i = 0; i < sizeof(happy) / 4; i++)
    {
	    DataTemp = *ptr++;
	    DataTemp |= (*ptr++ << 8);
		DataTemp |= (*ptr++ << 16);
		DataTemp |= (*ptr++ << 24);
        ID0 ^= DataTemp;
    }

    ptr = (uint8_t *)free;
    for(i = 0; i < sizeof(free) / 4; i++)
    {
	    DataTemp = *ptr++;
	    DataTemp |= (*ptr++ << 8);
		DataTemp |= (*ptr++ << 16);
		DataTemp |= (*ptr++ << 24);
        ID1 ^= DataTemp;
    }

    ptr = (uint8_t *)MyWish;
    for(i = 0; i < sizeof(MyWish) / 4; i++)
    {
	    DataTemp = *ptr++;
	    DataTemp |= (*ptr++ << 8);
		DataTemp |= (*ptr++ << 16);
		DataTemp |= (*ptr++ << 24);
        ID2 ^= DataTemp;
    }

    temp = ID2;
    PtrTemp = (uint32_t *)FlashId;
    ID2 = ID1 - *PtrTemp++;
    ID1 = ID0 + *PtrTemp++;
    ID0 = temp ^ *PtrTemp;

    Buff[OFFSET_SECRET] = (uint8_t)ID2;
    Buff[OFFSET_SECRET + 1] = (uint8_t)(ID2 >> 8);
    Buff[OFFSET_SECRET + 2] = (uint8_t)(ID2 >> 16);
    Buff[OFFSET_SECRET + 3] = (uint8_t)(ID2 >> 24);

    Buff[OFFSET_SECRET + 4] = (uint8_t)ID1;
    Buff[OFFSET_SECRET + 5] = (uint8_t)(ID1 >> 8);
    Buff[OFFSET_SECRET + 6] = (uint8_t)(ID1 >> 16);
    Buff[OFFSET_SECRET + 7] = (uint8_t)(ID1 >> 24);

    Buff[OFFSET_SECRET + 8] = (uint8_t)ID0;
    Buff[OFFSET_SECRET + 9] = (uint8_t)(ID0 >> 8);
    Buff[OFFSET_SECRET + 10] = (uint8_t)(ID0 >> 16);
    Buff[OFFSET_SECRET + 11] = (uint8_t)(ID0 >> 24);
}





		   
void mDelay(int ms)
{
    volatile int i, j;
	

    for(i = 0; i < ms; i++)
	  {
			//IWDG_ReloadCounter();
        for(j = 0; j < 5000; j++);
		}
}

////TEA密钥采用隔一个写入的方式进行存储，中间参杂一个随机数
////其占用32字节的空间
//void MixTeaKey(int StartOffset)
//{
//    unsigned char TeaKey[16] = {0x41, 0x69, 0xf4, 0xa5, 
//                                                 0x8b, 0x91, 0x5c, 0x7d, 
//                                                 0x02, 0x5b, 0xc4, 0x12, 
//                                                 0xdc, 0x52, 0x27, 0x39};
//    volatile uint32_t ID0;          
//    int i;
//    ID0 = *(volatile uint32_t*)(ID0_ADDR);
//    
//    for(i = 0; i < 16 / 4; i++)
//    {
//        Buff[StartOffset + i * 8] = TeaKey[i * 4] ^ (ID0 >> 24);
//        Buff[StartOffset + i * 8 + 2] = TeaKey[i * 4 + 1] ^ (ID0 >> 16);
//        Buff[StartOffset + i * 8 + 4] = TeaKey[i * 4 + 2] ^ (ID0 >> 8);
//        Buff[StartOffset + i * 8 + 6] = TeaKey[i * 4 + 3] ^ ID0;
//    }
//    

//    
//}


uint32_t GetFlashAddr()
{
    //return BOOTLOADER_ADDR - FLASH_SECTOR_SIZE * 4;
    return NRF_FICR->CODESIZE * NRF_FICR->CODEPAGESIZE - NRF_FICR->CODEPAGESIZE;
    //return SECRET_ADDRESS;
}

/**
 * @brief Function for application main entry.
 */
int32_t SecretWrittenFlag;
int main(void)
{
	SetBuffRandom();

	FlashAddr = GetFlashAddr();

    MixerBuffWithID(Buff);

    MixRsaKey();
	
	MixFwRsaKey();
  //IWDG_ReloadCounter();

    MixAnotherPublicKey();
  //IWDG_ReloadCounter();


    MixChipId();
    //MixTeaKey(1900);
  //IWDG_ReloadCounter();

    uint8_t *FlashAddrU8 = (uint8_t *)FlashAddr;
    for(int i = 0; i < FLASH_SECTOR_SIZE; i++)
    {
        FlashContent[i] = *FlashAddrU8;
        FlashAddrU8++;
    }

    SecretWrittenFlag = memcmp(FlashContent, Buff, FLASH_SECTOR_SIZE);
    if(SecretWrittenFlag != 0)
    {
        flash_page_erase(&FlashAddr);


        for(int i = 0; i < FLASH_SECTOR_SIZE / 4; i++)
        {
            uint32_t DataTemp;
            memcpy(&DataTemp, (uint8_t *)(Buff + i * 4), 4);
            flash_word_write((uint32_t *)FlashAddr, DataTemp);
            FlashAddr += 4;
        }
    }
    
    while (true)
    {

    }
}



/** @} */
