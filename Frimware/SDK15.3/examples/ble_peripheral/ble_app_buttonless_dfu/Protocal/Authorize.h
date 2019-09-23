#ifndef __AUTHORIZE__
#define __AUTHORIZE__


typedef enum {NO_ALLOW = 0, AUDIO_DOWNLOAD_ALLOW = 1} AuthorizeTypeFlag;


struct AuthorizeInfo
{
    AuthorizeTypeFlag AuthorizeType;
    unsigned char WriteAudioGUID[16];
};


extern struct AuthorizeInfo Authorize;


int AuthorizeAudioDownloadInfo(unsigned char *buff);
void GetAuthorizeRSAPulbicKey(void);
int AuthorizeAudioGuid(unsigned char *EncyData);

#endif



