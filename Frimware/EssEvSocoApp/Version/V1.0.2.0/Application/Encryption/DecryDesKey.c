#include "common.h"
#include "Rsa.h"
#include "Rsaeuro.h"
#include "R_Random.h"
#include "DecryDesKey.h"
#include "Tea.h"
#include "SystemInfo.h"


void GetRsaKeyContent(unsigned char *buff, int cnt, int offset, unsigned char *DataBuff)
{
    int i;
    vu32 ID2;
    ID2 = *(vu32*)(ID2_ADDR);
    
    for(i = 0; i < cnt; i++)
    {
        buff[i] = 0;
        buff[cnt + i] = DataBuff[offset + i];
    }

    for(i = 0; i < (cnt >> 2); i++)
    {
        buff[cnt + i * 4] = buff[cnt +  i * 4] ^ (ID2 >> 24);
        buff[cnt + i * 4 + 1] = buff[cnt + i * 4 + 1] ^ (ID2 >> 16);
        buff[cnt + i * 4 + 2] = buff[cnt + i * 4 + 2] ^ (ID2 >> 8);
        buff[cnt + i * 4 + 3] = buff[cnt + i * 4 + 3] ^ ID2;
    }
}

R_RSA_PRIVATE_KEY privateKey;
void GetRsaPrivateKey()
{
    unsigned char *DataPtr;
    int i;
    unsigned char RadomDataBuff[1024];
    


    
    DataPtr = (unsigned char *)(SECURE_START_ADDR);
    for(i = 0; i < 1024; i++)
    {
        RadomDataBuff[i] = *DataPtr;
        DataPtr++;
    }
    
    privateKey.bits = 512;
    GetRsaKeyContent(privateKey.modulus, 64, OFFSET_MODULUS, RadomDataBuff);
    GetRsaKeyContent(privateKey.publicExponent, 64, OFFSET_PUBLIC_EXPONENT, RadomDataBuff);
    GetRsaKeyContent(privateKey.exponent, 64, OFFSET_EXPONENT, RadomDataBuff);
    GetRsaKeyContent(privateKey.prime[0], 32, OFFSET_PRIME0, RadomDataBuff);
    GetRsaKeyContent(privateKey.prime[1], 32, OFFSET_PRIME1, RadomDataBuff);
    GetRsaKeyContent(privateKey.primeExponent[0], 32, OFFSET_PRIME_EXPONENT0, RadomDataBuff);
    GetRsaKeyContent(privateKey.primeExponent[1], 32, OFFSET_PRIME_EXPONENT1, RadomDataBuff);
    GetRsaKeyContent(privateKey.coefficient, 32, OFFSET_COEFFICIENT, RadomDataBuff);
}


int  DecryTeaKey(unsigned char *CrypedDesKey, unsigned int *DesKey)
{
    unsigned char DesBuff[256];
    unsigned int decryptedLength;
    int i;
    int status;
    //GetRsaPrivateKey();

    for(i = 0; i < 64; i++)
    {
        CrypedDesKey[i] = CrypedDesKey[i] - 0x5A;
    }

    status = RSAPrivateDecrypt(DesBuff, &decryptedLength, CrypedDesKey, 64, &privateKey);
    if(status != 0 || decryptedLength != 16)
    {
        return 53;
    }

    for(i = 0; i < 4; i++)
    {
        DesKey[i] = (DesBuff[i * 4] | (DesBuff[i * 4 + 1] << 8) | (DesBuff[i * 4 + 2] << 16) | (DesBuff[i * 4 + 3] << 24));
    }
    return 0;
}


