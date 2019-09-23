#ifndef __TEA__
#define __TEA__

void encrypt(unsigned long *v, unsigned long *k);
void decrypt(unsigned int *v, unsigned int *k);

 void DecryptContent(uint8_t *buff, uint32_t length);
 void DecryptAndDeconfuse(uint8_t *buff, uint32_t length);
 void ConfuseWav_16bit(uint8_t *buff, uint32_t length);
 void DeconfuseWav_16bit(uint8_t *buff, uint32_t length);

extern unsigned int TeaKeyTable[4];

#endif

