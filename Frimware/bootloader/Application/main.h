#ifndef __MAIN__
#define __MAIN__

//#define ESS_ONE_PLUS_PRODUCT_ID    							0x010A
#define AES_OFFSET											130


#define OFFSET_FW_MODULUS										700
#define OFFSET_FW_PUBLIC_EXPONENT								780
#define OFFSET_FW_EXPONENT										1021
#define OFFSET_FW_PRIME0										1341			
#define OFFSET_FW_PRIME1										1451
#define OFFSET_FW_PRIME_EXPONENT0								1521
#define OFFSET_FW_PRIME_EXPONENT1								1600
#define OFFSET_FW_COEFFICIENT									1690






//使用外部晶振
//#define USE_HSE
//使用内部晶振
//#define USE_HSI

typedef enum {FIRMWARE_COMPLETE = 0, FIRMWARE_BROKEN = 0xFF} FirmwareCompleteFlagType;
#define FIRMWARE_COMPLETE_FLAG_OFFSET			512



#define APPLICATION_ADDRESS				0x08004000
#define OnChipAudioMenuAddr				0x0803E000   
#define SYSTEM_INFO_ADDR				0x0803F000
#define SECURE_START_ADDR				0x0803F800






#define UNIQUE_ID_ADDR_051			    0x1FFFF7E8//0x1FFFF7AC
#define ID0_ADDR						(UNIQUE_ID_ADDR_051)
#define ID1_ADDR						(UNIQUE_ID_ADDR_051 + 4)
#define ID2_ADDR						(UNIQUE_ID_ADDR_051 + 8)


typedef signed long  s32;
typedef signed short s16;
typedef signed char  s8;

typedef signed long  const sc32;  /* Read Only */
typedef signed short const sc16;  /* Read Only */
typedef signed char  const sc8;   /* Read Only */

typedef volatile signed long  vs32;
typedef volatile signed short vs16;
typedef volatile signed char  vs8;

typedef volatile signed long  const vsc32;  /* Read Only */
typedef volatile signed short const vsc16;  /* Read Only */
typedef volatile signed char  const vsc8;   /* Read Only */

typedef unsigned long  u32;
typedef unsigned short u16;
typedef unsigned char  u8;

typedef unsigned long  const uc32;  /* Read Only */
typedef unsigned short const uc16;  /* Read Only */
typedef unsigned char  const uc8;   /* Read Only */

typedef volatile unsigned long  vu32;
typedef volatile unsigned short vu16;
typedef volatile unsigned char  vu8;

typedef volatile unsigned long  const vuc32;  /* Read Only */
typedef volatile unsigned short const vuc16;  /* Read Only */
typedef volatile unsigned char  const vuc8;   /* Read Only */




#include "stm32f10x_lib.h"
#include <stdint.h>

#include "ActionTick.h"
#include "SystemHwconfig.h"

#include "ApplicationRelationship.h"

#include "aes.h"



#include "DecryDesKey.h"

#include "BleComHw.h"
#include "BleTransportLayer.h"
#include "BleDataLayer.h"
#include "BleError.h"
#include "FirmwareConvert.h"
#include "SpiFlash.h"
#include "SystemInfo.h"
#include "Downloader.h"



#endif


