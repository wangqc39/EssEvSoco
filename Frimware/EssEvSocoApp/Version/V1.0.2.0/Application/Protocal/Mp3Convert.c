#include "common.h"
#include <stdint.h>
#include <string.h>
#include "SpiFlash.h"
#include "mp3dec.h"
#include "tea.h"
#include "fs.h"

#define FR_OK 		0
typedef unsigned int	UINT;
typedef unsigned char	BYTE;

#define f_tell()		(Mp3File.ReadHeadSeek)

#define FILE_READ_BUFFER_SIZE 2048

static int8_t file_read_buffer[FILE_READ_BUFFER_SIZE];
HMP3Decoder				hMP3Decoder;

struct Mp3FileInfo
{
    uint32_t StartAddr;//读取的地址，FLASH的物理地址
    uint32_t FileLength;
    uint32_t ReadSeek;
    uint32_t ReadHeadSeek;
};

struct Mp3FileInfo Mp3File;

int32_t OpenMp3File(uint32_t StartAddr, uint32_t FileLength)
{
    if(StartAddr >= SpiFlash.FlashSize)
        return -1;
    Mp3File.StartAddr = StartAddr;
    Mp3File.FileLength = FileLength;
    Mp3File.ReadSeek = 0;
    Mp3File.ReadHeadSeek = 0;
    return 0;
}



//头部的Seek，头部解析时使用，因为头部数据需要先读取，解密后才能解析
int32_t Mp3HeadSeek(uint32_t seek)
{
    if(seek >= FILE_READ_BUFFER_SIZE)
        return -1;
    Mp3File.ReadHeadSeek = seek;
    return 0;
}

//头部的读取，读取file_read_buffer中的内容，因为MP3原始数据被加密过，直接通过Mp3Read，无法进行解密
int32_t Mp3HeadRead(uint8_t *buff, uint32_t btr, uint32_t *br)
{
    uint32_t ThisReadCnt;
    //uint32_t ReadCnt;
    if(Mp3File.ReadHeadSeek >= FILE_READ_BUFFER_SIZE)
    {
        *br = 0;
        return 0;
    }
        
    
    if(Mp3File.ReadHeadSeek + btr <= FILE_READ_BUFFER_SIZE)
    {
        ThisReadCnt = btr;
    }
    else
    {
        //文件要读完了，剩余数据量不足btr字节
        ThisReadCnt = FILE_READ_BUFFER_SIZE - Mp3File.ReadHeadSeek;
    }

    memcpy(buff, file_read_buffer + Mp3File.ReadHeadSeek, ThisReadCnt);
    Mp3File.ReadHeadSeek += ThisReadCnt;

    *br = ThisReadCnt;

    return 0;
}

//设定文件读取的Seek
int32_t Mp3Seek(uint32_t seek)
{
    if(seek >= Mp3File.FileLength)
        return -1;
    Mp3File.ReadSeek = seek;
    return 0;
}


//文件内容的读取操作
int32_t Mp3Read(uint8_t *buff, uint32_t btr, uint32_t *br)
{
    uint32_t ThisReadCnt;
    //uint32_t ReadCnt;
    //文件已经读完了
    if(Mp3File.ReadSeek >= Mp3File.FileLength)
    {
        *br = 0;
        return 0;
    }
        
    
    if(Mp3File.ReadSeek + btr <= Mp3File.FileLength)
    {
        ThisReadCnt = btr;
    }
    else
    {
        //文件要读完了，剩余数据量不足btr字节
        ThisReadCnt = Mp3File.FileLength - Mp3File.ReadSeek;
    }

    DataFlashReadData(Mp3File.StartAddr + Mp3File.ReadSeek, buff, ThisReadCnt);
    Mp3File.ReadSeek += ThisReadCnt;
    *br = ThisReadCnt;

    return 0;
}

static uint32_t Mp3ReadId3V2Text(uint32_t unDataLen, char* pszBuffer, uint32_t unBufferSize)
{
	UINT unRead = 0;
	BYTE byEncoding = 0;
	if((Mp3HeadRead(&byEncoding, 1, &unRead) == FR_OK) && (unRead == 1))
	{
		unDataLen--;
		if(unDataLen <= (unBufferSize - 1))
		{
			if((Mp3HeadRead((uint8_t *)pszBuffer, unDataLen, &unRead) == FR_OK) ||
					(unRead == unDataLen))
			{
				if(byEncoding == 0)
				{
					// ISO-8859-1 multibyte
					// just add a terminating zero
					pszBuffer[unDataLen] = 0;
				}
				else if(byEncoding == 1)
				{
					// UTF16LE unicode
					uint32_t r = 0;
					uint32_t w = 0;
					if((unDataLen > 2) && (pszBuffer[0] == 0xFF) && (pszBuffer[1] == 0xFE))
					{
						// ignore BOM, assume LE
						r = 2;
					}
					for(; r < unDataLen; r += 2, w += 1)
					{
						// should be acceptable for 7 bit ascii
						pszBuffer[w] = pszBuffer[r];
					}
					pszBuffer[w] = 0;
				}
			}
			else
			{
				return 1;
			}
		}
		else
		{
			// we won't read a partial text
			if(Mp3HeadSeek(f_tell() + unDataLen) != FR_OK)
			{
				return 1;
			}
		}
	}
	else
	{
		return 1;
	}
	return 0;
}

static uint32_t Mp3ReadId3V2Tag( char* pszArtist, uint32_t unArtistSize, char* pszTitle, uint32_t unTitleSize)
{
	pszArtist[0] = 0;
	pszTitle[0] = 0;

	BYTE id3hd[10];
	UINT unRead = 0;
	if((Mp3HeadRead(id3hd, 10, &unRead) != FR_OK) || (unRead != 10))
	{
		return 1;
	}
	else
	{
		uint32_t unSkip = 0;
		if((unRead == 10) &&
				(id3hd[0] == 'I') &&
				(id3hd[1] == 'D') &&
				(id3hd[2] == '3'))
		{
			unSkip += 10;
			unSkip = ((id3hd[6] & 0x7f) << 21) | ((id3hd[7] & 0x7f) << 14) | ((id3hd[8] & 0x7f) << 7) | (id3hd[9] & 0x7f);

			// try to get some information from the tag
			// skip the extended header, if present
			uint8_t unVersion = id3hd[3];
			if(id3hd[5] & 0x40)
			{
				BYTE exhd[4];
				Mp3HeadRead(exhd, 4, &unRead);
				size_t unExHdrSkip = ((exhd[0] & 0x7f) << 21) | ((exhd[1] & 0x7f) << 14) | ((exhd[2] & 0x7f) << 7) | (exhd[3] & 0x7f);
				unExHdrSkip -= 4;
				if(Mp3HeadSeek(f_tell() + unExHdrSkip) != FR_OK)
				{
					return 1;
				}
			}
			uint32_t nFramesToRead = 2;
			while(nFramesToRead > 0)
			{
				char frhd[10];
				if((Mp3HeadRead((uint8_t *)frhd, 10, &unRead) != FR_OK) || (unRead != 10))
				{
					return 1;
				}
				if((frhd[0] == 0) || (strncmp(frhd, "3DI", 3) == 0))
				{
					break;
				}
				char szFrameId[5] = {0, 0, 0, 0, 0};
				memcpy(szFrameId, frhd, 4);
				uint32_t unFrameSize = 0;
				uint32_t i = 0;
				for(; i < 4; i++)
				{
					if(unVersion == 3)
					{
						// ID3v2.3
						unFrameSize <<= 8;
						unFrameSize += frhd[i + 4];
					}
					if(unVersion == 4)
					{
						// ID3v2.4
						unFrameSize <<= 7;
						unFrameSize += frhd[i + 4] & 0x7F;
					}
				}

				if(strcmp(szFrameId, "TPE1") == 0)
				{
					// artist
					if(Mp3ReadId3V2Text(unFrameSize, pszArtist, unArtistSize) != 0)
					{
						break;
					}
					nFramesToRead--;
				}
				else if(strcmp(szFrameId, "TIT2") == 0)
				{
					// title
					if(Mp3ReadId3V2Text(unFrameSize, pszTitle, unTitleSize) != 0)
					{
						break;
					}
					nFramesToRead--;
				}
				else
				{
					if(Mp3HeadSeek(f_tell() + unFrameSize) != FR_OK)
					{
						return 1;
					}
				}
			}
		}
		if(Mp3HeadSeek(unSkip) != FR_OK)
		{
			return 1;
		}
	}

	return 0;
}

int32_t bytes_left;//表示file_read_buffer缓冲中剩余数据的数量
int8_t *read_ptr;//file_read_buffer中读取的指针
//成功:返回数据的个数
int DecodeOneFrame(int16_t *DecodeBuff)
{
    int offset, err;
    MP3FrameInfo mp3FrameInfo;

    offset = MP3FindSyncWord((unsigned char*)read_ptr, bytes_left);
    bytes_left -= offset;
    read_ptr += offset;

    err = MP3Decode(hMP3Decoder, (unsigned char**)&read_ptr, (int*)&bytes_left, DecodeBuff, 0);
    if (err) {
		/* error occurred */
		switch (err) {
		case ERR_MP3_INDATA_UNDERFLOW:
			return -1;
		case ERR_MP3_MAINDATA_UNDERFLOW:
			/* do nothing - next call to decode will provide more mainData */
			return 0;
		case ERR_MP3_FREE_BITRATE_SYNC:
		default:
			return -1;
		}
	} else {
		// no error
		MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
		return mp3FrameInfo.outputSamps;
	}
}

// 1.对MP3数据进行读取，TEA解密及去混淆，得到MP3原始数据
// 2.对MP3数据进行解码，变成SND数据，并增加混淆
// 3.实时写入到音频区域中

//StartAddr为MP3文件起始地址的物理地址
//FileLength，MP3文件的长度
//返回解码后，WAV文件的总长度
int32_t DecryptionConvertMp3(struct FileInfo *DistFp, uint32_t StartAddr, uint32_t FileLength)
{
    int32_t ret;
    uint32_t br;
    int32_t DecodeRes;
    int32_t WavFileLength = 0;
    //初始化当前的MP3文件
    ret = OpenMp3File(StartAddr, FileLength);
    if(ret != 0)
        return ret;

    char szArtist[120];
    char szTitle[120];
    //读取MP3头部，解密，对头部数据进行解析
    DataFlashReadData(Mp3File.StartAddr, (uint8_t *)file_read_buffer, FILE_READ_BUFFER_SIZE);
    DecryptAndDeconfuse((uint8_t *)file_read_buffer, FILE_READ_BUFFER_SIZE);
    ret = Mp3ReadId3V2Tag(szArtist, sizeof(szArtist), szTitle, sizeof(szTitle));
    if(ret != 0)
        return -40;

    

    //解析完成后，根据头部解析时的Seek，给定文件读取的Seek
    ret = Mp3Seek(Mp3File.ReadHeadSeek);

    
    Mp3Read((uint8_t *)file_read_buffer, FILE_READ_BUFFER_SIZE, &br);
    DecryptAndDeconfuse((uint8_t *)file_read_buffer, FILE_READ_BUFFER_SIZE);
    
    hMP3Decoder = MP3InitDecoder();



    int32_t res;
    static int16_t WriteBuff[576];//22050采样率
    bytes_left = FILE_READ_BUFFER_SIZE;
    read_ptr = file_read_buffer;
    while(1)
    {
        //判断MP3数据的缓冲中的数据是否已经转换超过了一般
        if(bytes_left < (FILE_READ_BUFFER_SIZE / 2))
        {
            //缓冲内数据已经转换超过了一半，从文件中补充一半的数据
            memcpy(file_read_buffer, file_read_buffer + (FILE_READ_BUFFER_SIZE / 2), (FILE_READ_BUFFER_SIZE / 2));
            read_ptr -= (FILE_READ_BUFFER_SIZE / 2);
    
            res = Mp3Read((uint8_t *)file_read_buffer + (FILE_READ_BUFFER_SIZE / 2), (FILE_READ_BUFFER_SIZE / 2), &br);
            DecryptAndDeconfuse((uint8_t *)file_read_buffer + (FILE_READ_BUFFER_SIZE / 2), br);
    
            bytes_left += br;
    
            if(br != (FILE_READ_BUFFER_SIZE / 2) || res != FR_OK)
            {
                //文件读取异常，或者文件读取结束，跳出本流程
                break;
            }

            
        }

        //对一帧MP3数据进行解码，并写入到Wav文件中
        DecodeRes = DecodeOneFrame(WriteBuff);
        if(DecodeRes > 0)
        {
            ConfuseWav_16bit((uint8_t *)WriteBuff, DecodeRes * 2);
            res = WriteFileWithAlloc(DistFp, (uint8_t *)WriteBuff, DecodeRes * 2);
            if(res != DecodeRes * 2)
            {
                return -24;
            }
            WavFileLength += res;
        }
        else if(DecodeRes < 0)
        {
            
            return -23;
        }
    }
    
    //将剩余的MP3的数据转换成WAV数据
    while(bytes_left > 0)
    {
        DecodeRes = DecodeOneFrame(WriteBuff);
        if(DecodeRes > 0)
        {
            ConfuseWav_16bit((uint8_t *)WriteBuff, DecodeRes * 2);
            res = WriteFileWithAlloc(DistFp, (uint8_t *)WriteBuff, DecodeRes * 2);
            if(res != DecodeRes * 2)
            {
                return -25;
            }
            WavFileLength += res;
        }
        else if(DecodeRes < 0)
        {
            //由于音源制作工具会对文件进行末尾对齐
            //所以MP3文件的尾部随机性地会被去掉1-2个字节，所以如果在最后区域返回MP3解码失败
            //则直接完成该文件的转换。
            break;
        }
    }

    uint32_t AlineByte;
    //检查是否512对齐
    AlineByte = DistFp->FileLength % 512;
    if(AlineByte != 0)
    {
        //计算缺少的字节数
        AlineByte = 512 - AlineByte;
        memset(WriteBuff, 0xFF, AlineByte);
        res = WriteFileWithAlloc(DistFp, (uint8_t *)WriteBuff, AlineByte);
        if(res != AlineByte)
        {
            return -26;
        }
    }
    
    return WavFileLength;
}





