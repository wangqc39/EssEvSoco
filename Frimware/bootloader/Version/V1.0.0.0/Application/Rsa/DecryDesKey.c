#include "main.h"
#include "Rsa.h"
#include "Rsaeuro.h"
#include "R_Random.h"


void GetRsaKeyContent(unsigned char *buff, int cnt, int offset, unsigned char *RadomDataBuff)
{
    int i;
    vu32 ID2;
    ID2 = *(vu32*)(ID2_ADDR);
    
    for(i = 0; i < cnt; i++)
    {
        buff[i] = 0;
        buff[cnt + i] = RadomDataBuff[offset + i];
    }

    for(i = 0; i < cnt / 4; i++)
    {
        buff[cnt + i * 4] = buff[cnt +  i * 4] ^ (ID2 >> 24);
        buff[cnt + i * 4 + 1] = buff[cnt + i * 4 + 1] ^ (ID2 >> 16);
        buff[cnt + i * 4 + 2] = buff[cnt + i * 4 + 2] ^ (ID2 >> 8);
        buff[cnt + i * 4 + 3] = buff[cnt + i * 4 + 3] ^ ID2;
    }
}

R_RSA_PRIVATE_KEY privateKey;
void GetRsaKey()
{
    unsigned char *DataPtr;
    int i;
    unsigned char RadomDataBuff[STM32_PAGE_SIZE];
    


    
    DataPtr = (unsigned char *)(SECURE_START_ADDR);
    for(i = 0; i < STM32_PAGE_SIZE; i++)
    {
        RadomDataBuff[i] = *DataPtr;
        DataPtr++;
    }
    
    privateKey.bits = 512;
    GetRsaKeyContent(privateKey.modulus, 64, OFFSET_FW_MODULUS, RadomDataBuff);
    GetRsaKeyContent(privateKey.publicExponent, 64, OFFSET_FW_PUBLIC_EXPONENT, RadomDataBuff);
    GetRsaKeyContent(privateKey.exponent, 64, OFFSET_FW_EXPONENT, RadomDataBuff);
    GetRsaKeyContent(privateKey.prime[0], 32, OFFSET_FW_PRIME0, RadomDataBuff);
    GetRsaKeyContent(privateKey.prime[1], 32, OFFSET_FW_PRIME1, RadomDataBuff);
    GetRsaKeyContent(privateKey.primeExponent[0], 32, OFFSET_FW_PRIME_EXPONENT0, RadomDataBuff);
    GetRsaKeyContent(privateKey.primeExponent[1], 32, OFFSET_FW_PRIME_EXPONENT1, RadomDataBuff);
    GetRsaKeyContent(privateKey.coefficient, 32, OFFSET_FW_COEFFICIENT, RadomDataBuff);
    
}

int DecryDesKey(unsigned char *CrypedDesKey, unsigned char *DesKey)
{
    unsigned char DesBuff[32];
    unsigned int decryptedLength;
    int i;
    GetRsaKey();

    for(i = 0; i < 64; i++)
    {
        CrypedDesKey[i] = CrypedDesKey[i] - 0x54;
    }

    RSAPrivateDecrypt(DesBuff, &decryptedLength, CrypedDesKey, 64, &privateKey);

    if(decryptedLength != 32)
        return -1;

    for(i = 0; i < decryptedLength; i++)
    {

        DesKey[i] = DesBuff[i];
    }

    return 0;
    
}


