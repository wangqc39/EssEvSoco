#include <stdint.h>
unsigned int TeaKeyTable[4];




//其中V为数据，长度8字节，K为密钥，长度16字节
void encrypt(unsigned long *v, unsigned long *k) 
{
    unsigned long y = v[0], z = v[1], sum = 0, i;         /* set up */
	unsigned long delta = 0x9e3779b9;                 /* a key schedule constant */
    unsigned long a = k[0], b = k[1], c = k[2], d = k[3];   /* cache key */
    for (i = 0; i < 32; i++) {                        /* basic cycle start */
         sum += delta;
         y += ((z << 4) + a) ^ (z + sum) ^ ((z >> 5) + b);
         z += ((y << 4) + c) ^ (y + sum) ^ ((y >> 5) + d);/* end cycle */
     }
     v[0] = y;
     v[1] = z;
 }

 void decrypt(unsigned int *v, unsigned int *k) 
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

 void DecryptContent(uint8_t *buff, uint32_t length)
{
    uint32_t i;
    uint32_t DecryptCnt = length / 8;

    for(i = 0; i < DecryptCnt; i++)
    {
        decrypt((unsigned int*)(buff + i * 8), TeaKeyTable);
    }
}

void DeconfuseMp3_16bit(uint8_t *buff, uint32_t length)
{
//    uint32_t i;
//    for(i = 0; i < length / 2; i++)
//    {
//        buff[i * 2] = buff[i * 2] - 0x31;
//        buff[i * 2 + 1] = buff[i * 2 + 1] ^ 0x55;
//    }
    uint8_t *DistPtr = buff + length;
    while(buff < DistPtr)
    {
        *buff = *buff - 0x31;
        buff++;
        *buff = *buff ^ 0x55;
        buff++;
    }
}

void DecryptAndDeconfuse(uint8_t *buff, uint32_t length)
{
    DecryptContent(buff, length);
    DeconfuseMp3_16bit(buff, length);
}


void ConfuseWav_16bit(uint8_t *buff, uint32_t length)
{
    uint8_t *DistPtr = buff + length;
    while(buff < DistPtr)
    {
        *buff = *buff - 0xD7;
        buff++;
        *buff = *buff ^ 0xAA;
        buff++;
    }   
}

void DeconfuseWav_16bit(uint8_t *buff, uint32_t length)
{
    uint8_t *DistPtr = buff + length;
    while(buff < DistPtr)
    {
        *buff = *buff + 0xD7;
        buff++;
        *buff = *buff ^ 0xAA;
        buff++;
    }
}


 

 
