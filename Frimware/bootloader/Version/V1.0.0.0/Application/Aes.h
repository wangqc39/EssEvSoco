#ifndef __AES__
#define __AES__

void InitAesKeyTable(unsigned char key);
void aesDecInit(void);
void aesDecrypt( unsigned char * buffer, unsigned char * chainBlock );




extern unsigned int RemainFileLength;
extern unsigned char AESKeyTable[32];

#endif

