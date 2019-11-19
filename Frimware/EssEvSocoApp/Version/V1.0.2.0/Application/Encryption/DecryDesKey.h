#ifndef __DECRY_DES_KEY__
#define __DECRY_DES_KEY__

#include "Rsaeuro.h"
int  DecryTeaKey(unsigned char *CrypedDesKey, unsigned int *DesKey);

void GetRsaPrivateKey(void);
void GetRsaKeyContent(unsigned char *buff, int cnt, int offset, unsigned char *DataBuff);


extern R_RSA_PRIVATE_KEY privateKey;

#endif
