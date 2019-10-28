// FirmwarePublic.cpp : 定义控制台应用程序的入口点。
//
#undef UNICODE // 如果你不知道什么意思，请不要修改
#include "stdafx.h"
//include <string.h>
#include <io.h>
#include <iostream>
#include <windows.h>
//#include <winuser.h>
#include <stdio.h>
#include <stdlib.h>
#include "Rsa\rsaeuro.h"
#include "Rsa\Rsa.h"
#include "Rsa\R_Random.h"
#include <dos.h>


//制作前请确认如下配置
//1、是否加密
//2、各个组成文件的名字
//3、各个组成文件相对应的地址

//加密标志，0不加密，1加密
//#define ENCRY_FLAG						1
//包含的文件数
//#define FILE_NUM				1

/*#define BOOTER_START_ADDR			0
#define BOOTLOADER_START_ADDR		0x1000
#define APP_START_ADDR					0x5000
#define ON_CHIP_AUDIO_START_ADDR		0xE800
#define STORAGE_START_ADDR				0xF400
#define SECRET_START_ADDR				0xFC00

#define UPDATER_ADDR					0x6000
#define UPDATER_BOOTLOADER_ADDR			0x6800*/
#define BOOTER_START_ADDR				0
#define MIXER_START_ADDR				0x4000
#define ON_CHIP_AUDIO_START_ADDR		0x3E000

#define SYSTEM_INFO_ADDR				0x3F000
#define SECURE_START_ADDR				0x3F800

#define FIRMWARE_ONLY			1
//#define WHOLE_PROJECT			2
//#define BOOTLOADER_UPDATER		3
//#define ESS_ONE_WHOLE_PROJECT	4


#ifdef FIRMWARE_ONLY
#define ENCRY_FLAG						1
#define FILE_NUM						1
#define FILE_NAME0				"EssEvAppSoco_v1.0.0.1.bin"
#define START_ADDR0				MIXER_START_ADDR
#define FILE_NAME1				"OnChipAudio0x08015000"
#define START_ADDR1				ON_CHIP_AUDIO_START_ADDR
#define FILE_NAME2				"OnChipAudio0x08015000"
#define START_ADDR2				ON_CHIP_AUDIO_START_ADDR
#define FILE_NAME3				"StorageData.bin"
#define START_ADDR3				SYSTEM_INFO_ADDR
#define FILE_NAME4				"StorageData.bin"
#define START_ADDR4				SYSTEM_INFO_ADDR
#define START_ADDR5				SECURE_START_ADDR
#endif

#ifdef ESS_ONE_WHOLE_PROJECT
#define ENCRY_FLAG						0
#define FILE_NUM				4
#define FILE_NAME0				"EssEvBootloaderMicorMax_V1.0.0.0.bin"
#define START_ADDR0				BOOTER_START_ADDR
#define FILE_NAME1				"EssEvAppMicorMax_v1.0.13.0.bin"
#define START_ADDR1				MIXER_START_ADDR
#define FILE_NAME2				"OnChipAudio0x0803E000.bin"
#define START_ADDR2				ON_CHIP_AUDIO_START_ADDR
#define FILE_NAME3				"SystemInfoArea0x0803F000.bin"
#define START_ADDR3				SYSTEM_INFO_ADDR
#define FILE_NAME4				"SystemInfoArea.bin"
#define START_ADDR4				SECURE_START_ADDR
#define START_ADDR5				SECURE_START_ADDR
#endif

#ifdef WHOLE_PROJECT
#define ENCRY_FLAG						0
#define FILE_NUM				4
#define FILE_NAME0				"EssOneBootloader.bin"
#define START_ADDR0				BOOTer_START_ADDR
#define FILE_NAME1				"EssOne.bin"
#define START_ADDR1				APP_START_ADDR
#define FILE_NAME2				"OnChipAudio0x08015000"
#define START_ADDR2				ON_CHIP_AUDIO_START_ADDR
#define FILE_NAME3				"StorageData.bin"
#define START_ADDR3				STORAGE_START_ADDR
#endif

#ifdef BOOTLOADER_UPDATER
#define ENCRY_FLAG						1
#define FILE_NUM						2
#define FILE_NAME0				"BootloaderUpdater.bin"
#define START_ADDR0				APP_START_ADDR
#define FILE_NAME1				"EssOneBootloader.bin"
#define START_ADDR1				UPDATER_ADDR
#define FILE_NAME2				""
#define START_ADDR2				ON_CHIP_AUDIO_START_ADDR
#define FILE_NAME3				"StorageData.bin"
#define START_ADDR3				STORAGE_START_ADDR
#endif

//每个文件的名字和对应的起始地址
/*#if ENCRY_FLAG == 1
#define FILE_NAME0				"EssOne.bin"
#define START_ADDR0				APP_START_ADDR
#define FILE_NAME1				"OnChipAudio0x08015000"
#define START_ADDR1				ON_CHIP_AUDIO_START_ADDR
#define FILE_NAME2				""
#define START_ADDR2				STORAGE_START_ADDR
#define FILE_NAME3				"StorageData.bin"
#define START_ADDR3				STORAGE_START_ADDR
#else 
#define FILE_NAME0				"EssOneBootloader.bin"
#define START_ADDR0				BOOTLOADER_START_ADDR
#define FILE_NAME1				"EssOne.bin"
#define START_ADDR1				APP_START_ADDR
#define FILE_NAME2				"OnChipAudio0x08015000"
#define START_ADDR2				ON_CHIP_AUDIO_START_ADDR
#define FILE_NAME3				"StorageData.bin"
#define START_ADDR3				STORAGE_START_ADDR
#endif*/

#define FILE0_SPACE				(START_ADDR1 - START_ADDR0)
#define FILE1_SPACE				(START_ADDR2 - START_ADDR1)
#define FILE2_SPACE				(START_ADDR3 - START_ADDR2)
#define FILE3_SPACE				(START_ADDR4 - START_ADDR3)
#define FILE4_SPACE				(START_ADDR5 - START_ADDR4)



#define MAX_RESULT 256
#define BPOLY 0x1b //!< Lower 8 bits of (x^8+x^4+x^3+x+1), ie. (x^4+x^3+x+1).
#define BLOCKSIZE 16 //!< Block size in number of bytes.

#define KEY_COUNT 3

#if KEY_COUNT == 1
  #define KEYBITS 128 //!< Use AES128.
#elif KEY_COUNT == 2
  #define KEYBITS 192 //!< Use AES196.
#elif KEY_COUNT == 3
  #define KEYBITS 256 //!< Use AES256.
#else
  #error Use 1, 2 or 3 keys!
#endif

#if KEYBITS == 128
  #define ROUNDS 10 //!< Number of rounds.
  #define KEYLENGTH 16 //!< Key length in number of bytes.
#elif KEYBITS == 192
  #define ROUNDS 12 //!< Number of rounds.
  #define KEYLENGTH 24 //!< // Key length in number of bytes.
#elif KEYBITS == 256
  #define ROUNDS 14 //!< Number of rounds.
  #define KEYLENGTH 32 //!< Key length in number of bytes.
#else
  #error Key must be 128, 192 or 256 bits!
#endif


FILE *SourceFile, *DistFile;

unsigned char *powTbl; //!< Final location of exponentiation lookup table.
unsigned char *logTbl; //!< Final location of logarithm lookup table.
unsigned char *sBox; //!< Final location of s-box.
unsigned char *sBoxInv; //!< Final location of inverse s-box.
unsigned char *expandedKey; //!< Final location of expanded key.

unsigned char block1[256]; //!< Workspace 1.
unsigned char block2[256]; //!< Worksapce 2.
unsigned char tempbuf[256];



unsigned char AES_Key_Table[32];





void CalcPowLog(unsigned char *powTbl, unsigned char *logTbl)
{
	unsigned char i = 0;
	unsigned char t = 1;
	
	do {
		// Use 0x03 as root for exponentiation and logarithms.
		powTbl[i] = t;
		logTbl[t] = i;
		i++;
		
		// Muliply t by 3 in GF(2^8).
		t ^= (t << 1) ^ (t & 0x80 ? BPOLY : 0);
	}while( t != 1 ); // Cyclic properties ensure that i < 255.
	
	powTbl[255] = powTbl[0]; // 255 = '-0', 254 = -1, etc.
}

void CalcSBox( unsigned char * sBox )
{
	unsigned char i, rot;
	unsigned char temp;
	unsigned char result;
	
	// Fill all entries of sBox[].
	i = 0;
	do {
		//Inverse in GF(2^8).
		if( i > 0 ) 
		{
			temp = powTbl[ 255 - logTbl[i] ];
		} 
		else 
		{
			temp = 0;
		}
		
		// Affine transformation in GF(2).
		result = temp ^ 0x63; // Start with adding a vector in GF(2).
		for( rot = 0; rot < 4; rot++ )
		{
			// Rotate left.
			temp = (temp<<1) | (temp>>7);
			
			// Add rotated byte in GF(2).
			result ^= temp;
		}
		
		// Put result in table.
		sBox[i] = result;
	} while( ++i != 0 );
}

void XORBytes( unsigned char * bytes1, unsigned char * bytes2, unsigned char count )
{
	do {
		*bytes1 ^= *bytes2; // Add in GF(2), ie. XOR.
		bytes1++;
		bytes2++;
	} while( --count );
}

void SubBytes( unsigned char * bytes, unsigned char count )
{
	do {
		*bytes = sBox[ *bytes ]; // Substitute every byte in state.
		bytes++;
	} while( --count );
}

void CycleLeft( unsigned char * row )
{
	// Cycle 4 bytes in an array left once.
	unsigned char temp = row[0];
	
	row[0] = row[1];
	row[1] = row[2];
	row[2] = row[3];
	row[3] = temp;
}


void KeyExpansion( unsigned char * expandedKey )
{
	unsigned char temp[4];
	unsigned char i;
	unsigned char Rcon[4] = { 0x01, 0x00, 0x00, 0x00 }; // Round constant.
	
	unsigned char * key = AES_Key_Table;
	
	// Copy key to start of expanded key.
	i = KEYLENGTH;
	do {
		*expandedKey = *key;
		expandedKey++;
		key++;
	} while( --i );
	
	// Prepare last 4 bytes of key in temp.
	expandedKey -= 4;
	temp[0] = *(expandedKey++);
	temp[1] = *(expandedKey++);
	temp[2] = *(expandedKey++);
	temp[3] = *(expandedKey++);
	
	// Expand key.
	i = KEYLENGTH;
	while( i < BLOCKSIZE*(ROUNDS+1) ) 
	{
		// Are we at the start of a multiple of the key size?
		if( (i % KEYLENGTH) == 0 )
		{
			CycleLeft( temp ); // Cycle left once.
			SubBytes( temp, 4 ); // Substitute each byte.
			XORBytes( temp, Rcon, 4 ); // Add constant in GF(2).
			*Rcon = (*Rcon << 1) ^ (*Rcon & 0x80 ? BPOLY : 0);
		}
		
		// Keysize larger than 24 bytes, ie. larger that 192 bits?
		#if KEYLENGTH > 24
		// Are we right past a block size?
		else if( (i % KEYLENGTH) == BLOCKSIZE ) {
		SubBytes( temp, 4 ); // Substitute each byte.
		}
		#endif
		
		// Add bytes in GF(2) one KEYLENGTH away.
		XORBytes( temp, expandedKey - KEYLENGTH, 4 );
		
		// Copy result to current 4 bytes.
		*(expandedKey++) = temp[ 0 ];
		*(expandedKey++) = temp[ 1 ];
		*(expandedKey++) = temp[ 2 ];
		*(expandedKey++) = temp[ 3 ];
		
		i += 4; // Next 4 bytes.
	}
}


void aesEncInit(void)
{
	powTbl = block1;
	logTbl = tempbuf;
	CalcPowLog( powTbl, logTbl );
	
	sBox = block2;
	CalcSBox( sBox );
	
	expandedKey = block1;
	KeyExpansion( expandedKey );
}




void ShiftRows( unsigned char * state )
{
	unsigned char temp;
	
	// Note: State is arranged column by column.
	
	// Cycle second row left one time.
	temp = state[ 1 + 0*4 ];
	state[ 1 + 0*4 ] = state[ 1 + 1*4 ];
	state[ 1 + 1*4 ] = state[ 1 + 2*4 ];
	state[ 1 + 2*4 ] = state[ 1 + 3*4 ];
	state[ 1 + 3*4 ] = temp;
	
	// Cycle third row left two times.
	temp = state[ 2 + 0*4 ];
	state[ 2 + 0*4 ] = state[ 2 + 2*4 ];
	state[ 2 + 2*4 ] = temp;
	temp = state[ 2 + 1*4 ];
	state[ 2 + 1*4 ] = state[ 2 + 3*4 ];
	state[ 2 + 3*4 ] = temp;
	
	// Cycle fourth row left three times, ie. right once.
	temp = state[ 3 + 3*4 ];
	state[ 3 + 3*4 ] = state[ 3 + 2*4 ];
	state[ 3 + 2*4 ] = state[ 3 + 1*4 ];
	state[ 3 + 1*4 ] = state[ 3 + 0*4 ];
	state[ 3 + 0*4 ] = temp;
}

unsigned char Multiply( unsigned char num, unsigned char factor )
{
	unsigned char mask = 1;
	unsigned char result = 0;
	
	while( mask != 0 ) 
	{
	// Check bit of factor given by mask.
		if( mask & factor ) 
		{
		  // Add current multiple of num in GF(2).
		  result ^= num;
		}
	
		// Shift mask to indicate next bit.
		mask <<= 1;
		
		// Double num.
		num = (num << 1) ^ (num & 0x80 ? BPOLY : 0);
	}
	
	return result;
}


unsigned char DotProduct( unsigned char * vector1, unsigned char * vector2 )
{
	unsigned char result = 0;
	
	result ^= Multiply( *vector1++, *vector2++ );
	result ^= Multiply( *vector1++, *vector2++ );
	result ^= Multiply( *vector1++, *vector2++ );
	result ^= Multiply( *vector1  , *vector2   );
	
	return result;
}

void MixColumn( unsigned char * column )
{
	unsigned char row[8] = {0x02, 0x03, 0x01, 0x01, 0x02, 0x03, 0x01, 0x01}; 
	// Prepare first row of matrix twice, to eliminate need for cycling.
	
	unsigned char result[4];
	
	// Take dot products of each matrix row and the column vector.
	result[0] = DotProduct( row+0, column );
	result[1] = DotProduct( row+3, column );
	result[2] = DotProduct( row+2, column );
	result[3] = DotProduct( row+1, column );
	
	// Copy temporary result to original column.
	column[0] = result[0];
	column[1] = result[1];
	column[2] = result[2];
	column[3] = result[3];
}

void MixColumns( unsigned char * state )
{
	MixColumn( state + 0*4 );
	MixColumn( state + 1*4 );
	MixColumn( state + 2*4 );
	MixColumn( state + 3*4 );
}

void Cipher( unsigned char * block, unsigned char * expandedKey )
{
	unsigned char round = ROUNDS-1;
	
	XORBytes( block, expandedKey, 16 );
	expandedKey += BLOCKSIZE;
	
	do {
		SubBytes( block, 16 );
		ShiftRows( block );
		MixColumns( block );
		XORBytes( block, expandedKey, 16 );
		expandedKey += BLOCKSIZE;
	} while( --round );
	
	SubBytes( block, 16 );
	ShiftRows( block );
	XORBytes( block, expandedKey, 16 );
}

void CopyBytes( unsigned char * to, unsigned char * from, unsigned char count )
{
	do {
		*to = *from;
		to++;
		from++;
	} while( --count );
}

void aesEncrypt( unsigned char * buffer, unsigned char * chainBlock )
{
	XORBytes( buffer, chainBlock, BLOCKSIZE );
	Cipher( buffer, expandedKey );
	CopyBytes( chainBlock, buffer, BLOCKSIZE );
}

void GetDesKey();
void EncDesKey(unsigned char *MixerBuff);

int EncryptOneFile(char SourceFileName[FILE_NUM][1000])
{
	errno_t ret;
	char DistFileName[300];
	size_t ReadRet;
	unsigned char SourceReadBuff[16];
	//unsigned char DistWriteBuff[16];
	//int FileSize;
	unsigned char chainCipherBlock[16];
	unsigned char MixerBuff[512];
	int FileCnt = 0;

	strcpy_s(DistFileName, SourceFileName[0]);
	strcat_s(DistFileName, ".enc");

	ret = fopen_s(&SourceFile, SourceFileName[0], "rb");
	if(ret != 0)
	{
		printf("%s 无法打开\n", SourceFileName[0]);
		return 1;
	}

	ret = fopen_s(&DistFile, DistFileName, "wb");
	if(ret != 0)
	{
		fclose(SourceFile);
		return 1;
	}

    //FileSize = filelength(fileno(SourceFile));
	//InitAesKeyTable((unsigned char)(FileSize));

	//获取DES的密钥
	GetDesKey();
    //对DES密钥进行加密
	EncDesKey(MixerBuff);

	fwrite(MixerBuff, sizeof(char), 512, DistFile);

	//读取首个16字节数据
	ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
	if(ReadRet != 16)
	{
		fclose(SourceFile);
	    fclose(DistFile);
		return 1;
	}

	//16个字节一组的数据进行读取及加密
	do
	{
		//memset(DistWriteBuff, 0, 16);
		memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock));

		aesEncInit();//在执行加密初始化之前可以为AES_Key_Table赋值有效的密码数据
		aesEncrypt(SourceReadBuff, chainCipherBlock);//AES加密，数组dat里面的新内容就是加密后的数据。

		fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
		FileCnt += 16;
		ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
	}while(ReadRet == 16);
	//将剩下不满16字节的数据进行读取与加密
	if(ReadRet != 0)
	{
		int EmptyCnt;
		int k;
		EmptyCnt = 16 - ReadRet;
		for(k = 0; k < EmptyCnt; k++)
		{
			SourceReadBuff[ReadRet + k] = 0xFF;
		}
		memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock));

		aesEncInit();//在执行加密初始化之前可以为AES_Key_Table赋值有效的密码数据
		aesEncrypt(SourceReadBuff, chainCipherBlock);//AES加密，数组dat里面的新内容就是加密后的数据。

	    fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
		FileCnt += 16;
	}

	
	if(FileCnt > FILE0_SPACE)
	{
		printf("%s大小超过了%d！\n", FILE_NAME0, FILE0_SPACE);
	}
	fclose(SourceFile);


	if(FILE_NUM >= 2)
	{
		//FILE *AudioFile;
		ret = fopen_s(&SourceFile, SourceFileName[1], "rb");
	    if(ret != 0)
		{
			printf("%s 无法打开\n", SourceFileName[1]);
		    return 1; 
		}
		//将文件填充至FILE0_SPACE
		while(FileCnt < FILE0_SPACE)
		{
			memset(SourceReadBuff, 0xFF, 16);

			memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock));
			aesEncInit();//在执行加密初始化之前可以为AES_Key_Table赋值有效的密码数据
			aesEncrypt(SourceReadBuff, chainCipherBlock);//AES加密，数组dat里面的新内容就是加密后的数据。
			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		//进行首个16字节的读取
		ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		if(ReadRet != 16)
		{
			fclose(SourceFile);
			fclose(DistFile);
			return 1;
		}
		//将16个字节一组的数据进行读取与加密
		do
		{
			//memset(DistWriteBuff, 0, 16);
			memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock));

			aesEncInit();//在执行加密初始化之前可以为AES_Key_Table赋值有效的密码数据
			aesEncrypt(SourceReadBuff, chainCipherBlock);//AES加密，数组dat里面的新内容就是加密后的数据。

			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;
			ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		}while(ReadRet == 16);
		//将剩下不满16字节的数据进行读取与加密
		if(ReadRet != 0)
		{
			int EmptyCnt;
			int k;
			EmptyCnt = 16 - ReadRet;
			for(k = 0; k < EmptyCnt; k++)
			{
				SourceReadBuff[ReadRet + k] = 0xFF;
			}
			memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock));

			aesEncInit();//在执行加密初始化之前可以为AES_Key_Table赋值有效的密码数据
			aesEncrypt(SourceReadBuff, chainCipherBlock);//AES加密，数组dat里面的新内容就是加密后的数据。

			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		if(FileCnt > FILE0_SPACE + FILE1_SPACE)
		{
			printf("%s大小超过了%d！\n", FILE_NAME1, FILE1_SPACE);
		}
		fclose(SourceFile);
	}

	if(FILE_NUM >= 3)
	{
		//FILE *AudioFile;
		ret = fopen_s(&SourceFile, SourceFileName[2], "rb");
	    if(ret != 0)
		{
			printf("%s 无法打开\n", SourceFileName[2]);
		    return 1; 
		}
		//将文件填充至FILE0_SPACE
		while(FileCnt < FILE0_SPACE + FILE1_SPACE)
		{
			memset(SourceReadBuff, 0xFF, 16);

			memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock));
			aesEncInit();//在执行加密初始化之前可以为AES_Key_Table赋值有效的密码数据
			aesEncrypt(SourceReadBuff, chainCipherBlock);//AES加密，数组dat里面的新内容就是加密后的数据。
			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		//进行首个16字节的读取
		ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		if(ReadRet != 16)
		{
			fclose(SourceFile);
			fclose(DistFile);
			return 1;
		}
		//将16个字节一组的数据进行读取与加密
		do
		{
			//memset(DistWriteBuff, 0, 16);
			memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock));

			aesEncInit();//在执行加密初始化之前可以为AES_Key_Table赋值有效的密码数据
			aesEncrypt(SourceReadBuff, chainCipherBlock);//AES加密，数组dat里面的新内容就是加密后的数据。

			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;
			ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		}while(ReadRet == 16);
		//将剩下不满16字节的数据进行读取与加密
		if(ReadRet != 0)
		{
			int EmptyCnt;
			int k;
			EmptyCnt = 16 - ReadRet;
			for(k = 0; k < EmptyCnt; k++)
			{
				SourceReadBuff[ReadRet + k] = 0xFF;
			}
			memset(chainCipherBlock, 0x00, sizeof(chainCipherBlock));

			aesEncInit();//在执行加密初始化之前可以为AES_Key_Table赋值有效的密码数据
			aesEncrypt(SourceReadBuff, chainCipherBlock);//AES加密，数组dat里面的新内容就是加密后的数据。

			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		if(FileCnt > FILE0_SPACE + FILE1_SPACE + FILE2_SPACE)
		{
			printf("%s大小超过了%d！\n", FILE_NAME2, FILE2_SPACE);
		}
		fclose(SourceFile);
	}

	
	fclose(DistFile);
	return 0;
}

int CombineOneFile(char SourceFileName[FILE_NUM][1000])
{
	errno_t ret;
	char DistFileName[300];
	size_t ReadRet;
	unsigned char SourceReadBuff[16];
	//unsigned char DistWriteBuff[16];
	//int FileSize;
	unsigned char chainCipherBlock[16];
	unsigned char MixerBuff[512];
	int FileCnt = 0;

	strcpy_s(DistFileName, SourceFileName[0]);
	strcat_s(DistFileName, ".cmb");

	ret = fopen_s(&SourceFile, SourceFileName[0], "rb");
	if(ret != 0)
	{
		printf("%s 无法打开\n", SourceFileName[0]);
		return 1;
	}

	ret = fopen_s(&DistFile, DistFileName, "wb");
	if(ret != 0)
	{
		fclose(SourceFile);
		return 1;
	}

	//读取首个16字节数据
	ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
	if(ReadRet != 16)
	{
		fclose(SourceFile);
	    fclose(DistFile);
		return 1;
	}

	//16个字节一组的数据进行读取及加密
	do
	{
		fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
		FileCnt += 16;
		ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
	}while(ReadRet == 16);
	//将剩下不满16字节的数据进行读取与加密
	if(ReadRet != 0)
	{
		int EmptyCnt;
		int k;
		EmptyCnt = 16 - ReadRet;
		for(k = 0; k < EmptyCnt; k++)
		{
			SourceReadBuff[ReadRet + k] = 0xFF;
		}
	    fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
		FileCnt += 16;
	}

	
	if(FileCnt > FILE0_SPACE)
	{
		printf("%s大小超过了%d！\n", FILE_NAME0, FILE0_SPACE);
	}
	fclose(SourceFile);


	if(FILE_NUM >= 2)
	{
		//FILE *AudioFile;
		ret = fopen_s(&SourceFile, SourceFileName[1], "rb");
	    if(ret != 0)
		{
			printf("%s 无法打开\n", SourceFileName[1]);
		    return 1; 
		}
		//将文件填充至FILE0_SPACE
		while(FileCnt < FILE0_SPACE)
		{
			memset(SourceReadBuff, 0xFF, 16);
			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		//进行首个16字节的读取
		ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		if(ReadRet != 16)
		{
			fclose(SourceFile);
			fclose(DistFile);
			return 1;
		}
		//将16个字节一组的数据进行读取与加密
		do
		{
			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;
			ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		}while(ReadRet == 16);
		//将剩下不满16字节的数据进行读取与加密
		if(ReadRet != 0)
		{
			int EmptyCnt;
			int k;
			EmptyCnt = 16 - ReadRet;
			for(k = 0; k < EmptyCnt; k++)
			{
				SourceReadBuff[ReadRet + k] = 0xFF;
			}
			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		if(FileCnt > FILE0_SPACE + FILE1_SPACE)
		{
			printf("%s大小超过了%d！\n", FILE_NAME1, FILE1_SPACE);
		}
		fclose(SourceFile);
	}

	if(FILE_NUM >= 3)
	{
		//FILE *AudioFile;
		ret = fopen_s(&SourceFile, SourceFileName[2], "rb");
	    if(ret != 0)
		{
			printf("%s 无法打开\n", SourceFileName[2]);
		    return 1; 
		}
		//将文件填充至FILE0_SPACE
		while(FileCnt < FILE0_SPACE + FILE1_SPACE)
		{
			memset(SourceReadBuff, 0xFF, 16);
			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		//进行首个16字节的读取
		ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		if(ReadRet != 16)
		{
			fclose(SourceFile);
			fclose(DistFile);
			return 1;
		}
		//将16个字节一组的数据进行读取与加密
		do
		{
			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;
			ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		}while(ReadRet == 16);
		//将剩下不满16字节的数据进行读取与加密
		if(ReadRet != 0)
		{
			int EmptyCnt;
			int k;
			EmptyCnt = 16 - ReadRet;
			for(k = 0; k < EmptyCnt; k++)
			{
				SourceReadBuff[ReadRet + k] = 0xFF;
			}

			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		if(FileCnt > FILE0_SPACE + FILE1_SPACE + FILE2_SPACE)
		{
			printf("%s大小超过了%d！\n", FILE_NAME2, FILE2_SPACE);
		}
		fclose(SourceFile);
	}

	if(FILE_NUM >= 4)
	{
		//FILE *AudioFile;
		ret = fopen_s(&SourceFile, SourceFileName[3], "rb");
	    if(ret != 0)
		{
			printf("%s 无法打开\n", SourceFileName[3]);
		    return 1; 
		}
		//将文件填充至FILE0_SPACE
		while(FileCnt < FILE0_SPACE + FILE1_SPACE + FILE2_SPACE)
		{
			memset(SourceReadBuff, 0xFF, 16);
			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		//进行首个16字节的读取
		ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		if(ReadRet != 16)
		{
			fclose(SourceFile);
			fclose(DistFile);
			return 1;
		}
		//将16个字节一组的数据进行读取与加密
		do
		{
			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;
			ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		}while(ReadRet == 16);
		//将剩下不满16字节的数据进行读取与加密
		if(ReadRet != 0)
		{
			int EmptyCnt;
			int k;
			EmptyCnt = 16 - ReadRet;
			for(k = 0; k < EmptyCnt; k++)
			{
				SourceReadBuff[ReadRet + k] = 0xFF;
			}

			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		if(FileCnt > FILE0_SPACE + FILE1_SPACE + FILE2_SPACE + FILE3_SPACE)
		{
			printf("%s大小超过了%d！\n", FILE_NAME3, FILE3_SPACE);
		}
		fclose(SourceFile);
	}

	if(FILE_NUM >= 5)
	{
		//FILE *AudioFile;
		ret = fopen_s(&SourceFile, SourceFileName[4], "rb");
	    if(ret != 0)
		{
			printf("%s 无法打开\n", SourceFileName[4]);
		    return 1; 
		}
		//将文件填充至FILE0_SPACE
		while(FileCnt < FILE0_SPACE + FILE1_SPACE + FILE2_SPACE + FILE3_SPACE)
		{
			memset(SourceReadBuff, 0xFF, 16);
			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		//进行首个16字节的读取
		ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		if(ReadRet != 16)
		{
			fclose(SourceFile);
			fclose(DistFile);
			return 1;
		}
		//将16个字节一组的数据进行读取与加密
		do
		{
			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;
			ReadRet = fread(SourceReadBuff, sizeof(char), 16, SourceFile);
		}while(ReadRet == 16);
		//将剩下不满16字节的数据进行读取与加密
		if(ReadRet != 0)
		{
			int EmptyCnt;
			int k;
			EmptyCnt = 16 - ReadRet;
			for(k = 0; k < EmptyCnt; k++)
			{
				SourceReadBuff[ReadRet + k] = 0xFF;
			}

			fwrite(SourceReadBuff, sizeof(char), 16, DistFile);
			FileCnt += 16;

		}
		if(FileCnt > FILE0_SPACE + FILE1_SPACE + FILE2_SPACE + FILE3_SPACE + FILE4_SPACE)
		{
			printf("%s大小超过了%d！\n", FILE_NAME3, FILE3_SPACE);
		}
		fclose(SourceFile);
	}

	
	fclose(DistFile);
	return 0;
}

R_RSA_PUBLIC_KEY publicKey;
unsigned char EncDesKeyBuff[64];

void GesAesKey()
{
	char modulus[128] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBA, 0x4C, 0x20, 0xAE, 0xA3, 0x89, 0xA1, 0x93, 0x2C, 0x4C, 0xEF, 0xB2, 0x12, 0xAC, 0x25, 0xD7, 0x35, 0x2A, 0x55, 0x8C, 0x24, 0xCA, 0x65, 0x37, 0x5E, 0x56, 0xCA, 0xCC, 0x15, 0xE9, 0x71, 0x75, 0xB7, 0x77, 0x2A, 0x6C, 0x0F, 0xBC, 0xDA, 0x6C, 0xA1, 0x99, 0xB8, 0x0F, 0xF8, 0x13, 0x82, 0xD3, 0x55, 0xC9, 0x9F, 0x6D, 0x41, 0xFD, 0x1A, 0x01, 0x09, 0x12, 0x13, 0xCF, 0xCD, 0x31, 0x4B, 0x25};
	char exponent[128] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01};
    int i;

	publicKey.bits = 512;
	for(i = 0; i < 128; i++)
	{
		publicKey.modulus[i] = modulus[i];
		publicKey.exponent[i] = exponent[i];
	}
}

//随机产生DES密钥
void GetDesKey()
{
	int i;
	R_RANDOM_STRUCT RadomStruct1, RadomStruct2;
    R_RandomCreate(&RadomStruct1);
	Sleep(1);
	R_RandomCreate(&RadomStruct2);
	for(i = 0; i < 16; i++)
	{
		AES_Key_Table[2 * i] = RadomStruct1.state[i];
		AES_Key_Table[2 * i + 1] = RadomStruct2.state[i];
	}
}


void EncDesKey(unsigned char *MixerBuff)
{
	R_RANDOM_STRUCT randomStruct;
	R_RANDOM_STRUCT RadomStruct[32];
	int i;


	//产生512字节的随机数
	for(i = 0; i < 32; i++)
	{
		R_RandomCreate(&RadomStruct[i]);
		for(int j = 0; j < 1000000; j++);
	}

	for(i = 0; i < 16; i++)
	{
		for(int k = 0; k < 32; k++)
		{
			MixerBuff[i * 32 + k] = RadomStruct[k].state[i];
		}
	}

	R_RandomCreate(&randomStruct);
	unsigned int encryptedLength;
	GesAesKey();
	RSAPublicEncrypt(EncDesKeyBuff, &encryptedLength, AES_Key_Table, 32, &publicKey, &randomStruct);


	for(i = 0; i < 64; i++)
	{
		MixerBuff[i + 130] = EncDesKeyBuff[i] + 0x54;
	}
}




int _tmain(int argc, _TCHAR* argv[])
{
	
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char pause;
	//const char pattern[100] = "\\*.bin";
	char result[MAX_RESULT][MAX_PATH];
	int i;
	int j;
	int ret;

	char ExeFilePath[1000]; 
	char FilePath[FILE_NUM][1000];


    //得到当前文件路径名 
	GetCurrentDirectory(1000,ExeFilePath);
	for(i = 0; i < FILE_NUM; i++)
	{
		strcpy_s(FilePath[i], ExeFilePath);
		strcat_s(FilePath[i], "\\");
	}
	strcat_s(FilePath[0], FILE_NAME0);
	if(FILE_NUM >= 2)
	{
		strcat_s(FilePath[1], FILE_NAME1);
	}
	if(FILE_NUM >= 3)
	{
		strcat_s(FilePath[2], FILE_NAME2);
	}
	if(FILE_NUM >= 4)
	{
		strcat_s(FilePath[3], FILE_NAME3);
	}
	if(FILE_NUM >= 5)
	{
		strcat_s(FilePath[4], FILE_NAME4);
	}


	//获取AES的密钥
	GesAesKey();
	

	if(ENCRY_FLAG == 1)
	{
        ret = EncryptOneFile(FilePath);
		if(ret == 0)
		{
		    printf("加密完成\n");
		}
		else
		{
			printf("失败");
		}
	}
	else
	{
		ret = CombineOneFile(FilePath);
		if(ret == 0)
		{
		    printf("制作完成\n");
		}
		else
		{
			printf("失败");
		}
	}

    

	
	scanf_s("%c", &pause);
	return 0;
}

