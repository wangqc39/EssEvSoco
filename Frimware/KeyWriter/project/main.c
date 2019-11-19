/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : main.c
* Author             : MCD Application Team
* Version            : V1.0
* Date               : 10/08/2007
* Description        : Main program body
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"

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

/* Private typedef -----------------------------------------------------------*/
typedef enum { FAILED = 0, PASSED = !FAILED} TestStatus;




//#define ESS_ONE_PLUS_PRODUCT_ID    							0x010A
//#define AES_OFFSET											86

#define OFFSET_MODULUS										1
#define OFFSET_PUBLIC_EXPONENT								68
#define OFFSET_EXPONENT										168
#define OFFSET_PRIME0										250			
#define OFFSET_PRIME1										301
#define OFFSET_PRIME_EXPONENT0								364
#define OFFSET_PRIME_EXPONENT1								402
#define OFFSET_COEFFICIENT									479


#define OFFSET_FW_MODULUS										700
#define OFFSET_FW_PUBLIC_EXPONENT								780
#define OFFSET_FW_EXPONENT										1021
#define OFFSET_FW_PRIME0										1341			
#define OFFSET_FW_PRIME1										1451
#define OFFSET_FW_PRIME_EXPONENT0								1521
#define OFFSET_FW_PRIME_EXPONENT1								1600
#define OFFSET_FW_COEFFICIENT									1690


#define OFFSET_SECRET												580




#define  SECRET_ADDRESS		(0x0803F800)
#define FLASH_PAGE_SIZE     (0x800)
unsigned char happy[] = {"Ess Ev"};//29
unsigned char free[] = {"Awesome!"};//31

#define UNIQUE_ID_BASE_ADDR			 	0x1FFFF7E8
#define ID0_ADDR									(UNIQUE_ID_BASE_ADDR)
#define ID1_ADDR									(UNIQUE_ID_BASE_ADDR + 4)
#define ID2_ADDR									(UNIQUE_ID_BASE_ADDR + 8)


typedef  void (*pFunction)(void);
unsigned int JumpAddress;
pFunction Jump_To_Application;





unsigned char WriteBuff[2048] = {119,63,102,69,111,174,92,134,76,141,85,148,187,133,196,114,154,123,160,225,169,209,153,217,136,199,11,185,17,191,3,66,10,52,33,207,42,79,25,88,34,94,16,55,117,61,103,70,110,54,118,158,77,141,85,148,92,134,194,140,180,124,161,107,170,209,153,218,137,225,143,185,18,192,4,201,10,159,19,56,207,42,216,26,223,35,95,16,81,0,62,9,68,110,54,119,35,77,44,84,
149,93,132,76,141,60,122,161,108,168,115,154,124,161,225,144,183,127,192,136,199,11,185,18,166,208,175,214,47,223,30,207,39,190,20,62,6,69,13,55,2,61,103,45,84,28,93,37,77,142,86,148,67,109,154,124,161,108,170,128,193,143,185,127,192,4,173,215,159,223,166,207,40,214,46,197,5,181,14,53,20,62,6,68,217,29,91,35,77,44,84,28,93,132,51,116,60,122,66,108,
168,115,154,98,135,82,144,183,128,193,137,199,118,160,222,166,208,175,214,158,223,31,182,14,190,21,197,7,69,13,55,226,36,210,45,85,29,93,35,77,19,60,120,67,106,51,115,59,122,161,82,145,89,128,98,135,199,144,183,102,167,111,173,215,159,221,166,207,149,189,21,198,5,181,53,20,62,210,43,92,36,78,45,84,104,51,113,60,120,67,106,169,87,129,96,135,79,144,184,128,
193,111,174,118,160,222,166,208,175,214,133,7,69,217,30,224,36,210,43,85,29,93,10,52,19,58,121,67,104,51,113,34,94,136,80,143,87,128,95,135,200,118,158,102,167,111,173,215,159,222,140,182,149,189,22,198,5,181,14,165,224,37,210,43,217,29,226,36,78,19,59,3,68,12,51,114,25,87,129,96,136,80,145,103,167,109,174,118,160,76,166,206,148,187,134,193,140,179,14,160,
21,172,209,156,218,28,224,37,185,17,191,3,66,10,52,19,59,207,42,104,26,88,34,94,41,80,143,61,103,70,110,54,119,158,102,167,86,148,92,134,194,141,180,150,186,108,170,209,153,218,162,225,169,211,44,192,4,199,11,184,19,59,3,68,214,26,223,33,95,41,81,25,87,8,68,110,54,117,61,103,70,109,174,93,132,76,141,148,187,134,193,115,154,124,161,225,170,209,153,218,
137,199,11,185,18,191,4,64,10,52,223,33,85,148,187,133,196,114,154,123,160,225,169,17,52,223,33,207,42,79,25,88,34,94,16,55,117,61,103,70,110,54,118,158,77,141,85,148,92,134,194,140,180,124,161,107,170,209,153,218,137,225,143,185,18,4,201,10,159,19,56,207,42,216,26,223,95,16,81,0,62,9,68,110,54,119,35,77,44,84,149,93,132,60,122,161,168,161,225,144,183,192,136,199,11,185,18,166,208,175,214,47,
223,30,207,39,190,20,62,2,61,103,45,84,28,93,37,148,67,109,53,154,124,161,108,193,137,199,143,185,127,192,4,173,215,159,223,166,207,40,214,46,197,5,181,14,62,6,68,217,29,91,35,77,44,84,28,93,132,51,122,66,108,168,115,154,98,135,82,144,183,128,193,137,199,118,160,222,166,208,175,214,158,223,31,182,14,190,21,197,7,69,13,55,226,210,45,85,29,93,35,77,
19,60,120,67,106,51,115,59,122,161,89,128,98,135,199,144,183,102,167,111,173,215,159,221,166,207,149,189,21,198,5,181,14,53,20,62,210,29,92,36,78,45,84,3,67,104,51,113,60,120,166,208,175,214,133,198,5,182,14,188,21,197,7,29,52,19,58,121,67,104,51,113,34,94,136,80,143,87,128,95,135,200,118,158,111,173,215,159,222,140,182,149,189,22,5,181,14,165,224,37,
210,43,217,29,226,68,12,51,114,60,120,41,80,25,87,129,96,136,80,145,61,103,167,109,174,118,160,76,166,206,148,187,134,193,140,179,14,160,172,209,156,218,28,224,37,185,17,191,3,66,10,52,19,59,207,42,104,26,88,34,41,80,143,61,103,70,110,54,119,86,148,92,134,194,141,180,150,186,108,170,209,153,218,162,225,184,19,59,3,68,214,26,223,33,95,41,81,25,87,8,
68,110,54,117,61,103,70,109,174,93,132,76,141,85,148,187,134,193,115,154,124,161,225,170,209,153,218,137,199,11,185,18,191,4,64,10,52,223,33,207,42,79,26,88,34,95,16,55,118,62,104,71,110,54,119,158,77,142,86,148,92,134,194,141,180,124,161,108,170,209,154,218,160,225,144,183,18,192,4,198,11,185,17,57,207,38,214,47,223,30,95,39,79,20,62,6,69,111,55,117,
61,103,44,84,149,93,132,77,142,86,148,187,119,63,102,69,111,174,92,134,76,141,85,148,187,133,196,114,123,160,225,169,209,153,217,136,199,11,185,17,191,3,66,10,52,223,33,207,42,79,25,88,34,94,16,55,117,61,103,70,110,54,118,158,77,141,85,148,92,134,194,140,180,124,161,107,170,209,153,218,137,225,143,185,18,192,4,201,10,159,19,56,207,42,216,26,223,35,95,16,81,0,62,9,68,110,54,119,35,77,44,84,
149,93,132,76,141,60,122,161,108,168,115,154,124,161,225,144,183,127,192,136,199,11,185,18,166,208,175,214,62,6,69,13,55,2,61,103,45,84,28,93,37,77,142,86,148,67,109,53,154,124,161,108,170,89,128,193,199,143,185,127,192,4,173,215,159,223,166,207,40,214,46,197,5,181,14,53,20,62,6,68,217,29,91,35,77,44,84,28,93,132,51,116,60,122,66,108,168,115,154,98,
135,82,144,183,128,193,137,199,118,160,222,166,208,175,214,158,223,31,182,14,190,21,197,7,69,13,55,226,36,210,45,85,29,93,35,77,19,60,120,67,106,51,115,59,122,161,82,145,89,128,98,135,199,144,183,102,167,111,173,215,159,221,166,207,149,189,21,198,5,181,14,53,20,62,210,43,217,29,92,36,78,45,84,3,67,104,51,113,60,120,67,106,169,87,129,96,135,79,128,193,
111,174,118,160,222,166,208,175,214,133,198,5,182,14,188,21,197,7,69,217,30,224,36,210,43,85,29,93,10,52,19,58,121,67,104,51,113,34,94,136,80,143,87,128,95,135,200,118,158,102,167,111,173,215,159,222,140,182,149,189,22,198,5,181,14,165,224,37,210,43,217,29,226,36,78,19,59,3,68,12,51,114,60,120,41,80,25,129,96,136,80,145,61,103,167,109,174,118,206,148,
187,134,193,140,179,14,160,21,172,209,156,218,28,224,37,185,17,191,3,66,10,52,19,59,207,42,104,26,88,34,94,41,80,143,61,103,70,110,54,119,158,102,167,86,148,92,134,194,141,180,150,186,108,170,209,153,218,162,225,169,211,44,192,4,11,184,19,59,3,68,214,26,223,33,95,41,81,25,87,8,68,110,54,117,61,103,70,109,174,93,132,76,141,85,148,187,134,193,115,154,
124,161,225,170,209,153,218,137,199,11,185,18,119,63,102,69,111,174,92,134,76,141,85,148,187,133,196,114,154,123,160,225,169,209,153,217,136,199,11,185,17,191,3,66,10,52,223,33,207,42,79,25,88,34,94,16,55,117,61,103,70,110,54,118,158,77,141,85,148,92,134,194,140,180,124,161,107,170,209,153,218,137,225,143,185,18,192,4,201,10,159,19,56,207,42,216,26,223,35,95,16,81,0,62,9,68,110,54,119,35,77,44,
84,149,93,132,76,141,60,122,161,108,168,115,154,124,161,225,144,183,127,192,136,199,11,185,18,166,208,175,214,47,223,30,207,39,190,20,62,6,69,13,55,2,61,103,45,84,28,93,37,77,142,86,148,67,109,154,124,161,108,170,89,128,193,137,199,143,185,127,192,4,173,215,159,223,166,207,40,214,46,197,5,181,14,53,20,62,6,68,217,29,91,35,77,44,84,28,93,132,51,116,
60,122,66,108,168,82,144,183,128,193,137,199,118,160,222,166,208,175,214,158,223,31,182,14,190,21,197,7,69,13,55,226,210,45,85,29,93,35,77,19,60,120,67,106,51,115,59,122,161,145,89,128,98,135,199,144,183,102,167,111,173,215,159,221,166,207,149,189,21,198,5,181,14,53,20,62,210,43,217,29,92,36,78,45,84,3,67,104,51,113,60,120,106,169,87,129,96,135,79,144,
184,128,193,111,174,118,160,222,166,208,175,214,133,198,5,182,14,188,21,197,7,69,217,30,224,36,210,43,85,29,93,10,52,19,58,121,67,104,51,113,34,94,136,80,143,87,128,95,135,200,118,158,102,167,111,173,215,159,222,140,182,149,189,22,198,5,181,14,165,224,37,210,43,217,29,226,36,78,19,59,3,68,12,51,114,60,120,41,80,25,87,129,96,136,80,145,61,103,167,109,
174,118,160,76,166,206,148,187,134,193,140,179,14,160,21,172,209,156,218,28,224,37,185,17,191,3,66,10,52,19,59,207,42,104,26,88,34,94,41,80,143,61,103,70,110,54,119,158,102,167,86,148,92,134,194,141,180,150,186,108,170,209,153,218,162,225,169,211,44,192,4,199,11,184,19,59,3,68,214,26,223,33,95,41,81,25,87,8,68,110,54,117,61,103,70,109,174,93,85,148,
187,134,193,115,154,124,161,225,170,209,153,218};

unsigned char modulus[64] = {0xC4, 0xD9, 0xF2, 0x02, 0xF2, 0xFB, 0xAB, 0x93, 0x2D, 0x09, 0xDF, 0xC4, 0xC0, 0xBB, 0x10, 0xB1, 0xFD, 0x04, 0x8A, 0xA7, 0xCC, 0xDF, 0x2D, 0x51, 0xED, 0x4A, 0x8B, 0xB0, 0xD0, 0x9C, 0x7A, 0x67, 0x69, 0x2B, 0x5F, 0xE8, 0x0A, 0xAD, 0xA9, 0x1C, 0x39, 0x05, 0x69, 0x9C, 0xCA, 0x77, 0xC4, 0x55, 0xD7, 0x4D, 0x58, 0x70, 0xEC, 0x80, 0xD9, 0x78, 0x99, 0xB1, 0xC3, 0x00, 0x5A, 0x59, 0x1C, 0xBD};
unsigned char publicExponent[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01}; 
unsigned char exponent[64] = {0x45, 0x3D, 0x94, 0x0E, 0xCD, 0x81, 0x10, 0x9C, 0xB8, 0x1E, 0xDD, 0x02, 0xCB, 0xF8, 0x00, 0xAB, 0x5A, 0xA0, 0x1A, 0x55, 0x63, 0x8E, 0xAC, 0xD6, 0x40, 0x6D, 0x14, 0x2D, 0xA1, 0x73, 0x28, 0xEA, 0x50, 0xD7, 0x52, 0x40, 0x18, 0x4D, 0x4B, 0xE8, 0x25, 0x73, 0xC8, 0xAD, 0x3F, 0xA0, 0x66, 0xB2, 0x9F, 0x3E, 0x93, 0xB5, 0x95, 0x0A, 0xB4, 0xFE, 0xA7, 0x12, 0xE7, 0x61, 0xFA, 0xEE, 0x2F, 0x81};
unsigned char prime1[32] = {0xEA, 0x2A, 0x2B, 0x97, 0xC3, 0x54, 0x2B, 0x36, 0x1B, 0x23, 0x38, 0x9A, 0x90, 0x06, 0x85, 0xD5, 0x60, 0x27, 0xE9, 0xCC, 0xF6, 0x34, 0x06, 0x04, 0x5A, 0x76, 0x93, 0x83, 0xBB, 0x7D, 0xA7, 0x95};
unsigned char prime2[32] = {0xD7, 0x35, 0x0D, 0xD9, 0xE9, 0x78, 0xCE, 0x76, 0x8F, 0x4A, 0xBC, 0x4D, 0x6E, 0x21, 0x64, 0xAB, 0xAA, 0x70, 0x84, 0x0C, 0xBE, 0x5E, 0xD6, 0xA3, 0x27, 0xB7, 0x0C, 0x9B, 0x33, 0x7D, 0x36, 0x89};
unsigned char primeExponent1[32] = {0x20, 0x34, 0x1E, 0x7C, 0xA7, 0xA3, 0x4A, 0xB2, 0x0D, 0x37, 0x61, 0xD0, 0x77, 0xE0, 0x3D, 0xC5, 0xA3, 0x8C, 0xB9, 0xAD, 0xB3, 0x6A, 0x62, 0x2D, 0x75, 0x17, 0x7B, 0xA0, 0x11, 0x47, 0xED, 0xA5}; 
unsigned char primeExponent2[32] = {0x67, 0x55, 0x46, 0x2D, 0x57, 0xF9, 0x75, 0xC9, 0x5F, 0xCC, 0x56, 0xD6, 0x27, 0x07, 0x49, 0xBB, 0x53, 0xBF, 0x6A, 0xE8, 0x63, 0x90, 0x4E, 0x27, 0x51, 0x2E, 0x87, 0x93, 0x0E, 0x0A, 0x15, 0x61};
unsigned char coefficient[32] = {0x7D, 0x27, 0xEA, 0xFC, 0x8B, 0xFD, 0xBB, 0xA9, 0x14, 0x72, 0x41, 0x96, 0x79, 0xF0, 0xB8, 0xF2, 0xB9, 0xFF, 0x3C, 0xC1, 0xFA, 0x1A, 0x54, 0x33, 0x7C, 0xA0, 0x3B, 0x06, 0x68, 0x91, 0xB6, 0x95};   

unsigned char FWmodulus[64] = {0xBA, 0x4C, 0x20, 0xAE, 0xA3, 0x89, 0xA1, 0x93, 0x2C, 0x4C, 0xEF, 0xB2, 0x12, 0xAC, 0x25, 0xD7, 0x35, 0x2A, 0x55, 0x8C, 0x24, 0xCA, 0x65, 0x37, 0x5E, 0x56, 0xCA, 0xCC, 0x15, 0xE9, 0x71, 0x75, 0xB7, 0x77, 0x2A, 0x6C, 0x0F, 0xBC, 0xDA, 0x6C, 0xA1, 0x99, 0xB8, 0x0F, 0xF8, 0x13, 0x82, 0xD3, 0x55, 0xC9, 0x9F, 0x6D, 0x41, 0xFD, 0x1A, 0x01, 0x09, 0x12, 0x13, 0xCF, 0xCD, 0x31, 0x4B, 0x25};
unsigned char FWpublicExponent[64] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01}; 
unsigned char FWexponent[64] = {0x21, 0x5F, 0x59, 0xCB, 0x6F, 0x08, 0x4C, 0xA5, 0x6C, 0x9E, 0x2B, 0xA2, 0x1E, 0xAE, 0x2F, 0xE9, 0x29, 0x60, 0xF8, 0x0A, 0x2E, 0xDD, 0x02, 0xDC, 0xB4, 0x4D, 0x9F, 0xEE, 0x87, 0x4E, 0x56, 0x40, 0x8D, 0xB4, 0x60, 0x13, 0x09, 0x85, 0x25, 0x58, 0xA7, 0x26, 0x2B, 0x64, 0xC5, 0x9B, 0x5F, 0xFC, 0xF3, 0x48, 0x6A, 0x3B, 0x55, 0xA5, 0x7F, 0xEF, 0xAA, 0x58, 0xE8, 0x04, 0xBC, 0xFE, 0x68, 0x01};
unsigned char FWprime1[32] = {0xEB, 0x20, 0x35, 0x3B, 0x41, 0xD1, 0xB2, 0x44, 0xB8, 0x1F, 0x9E, 0xAD, 0xFB, 0x1E, 0x00, 0x92, 0xE1, 0xB1, 0x0F, 0xB5, 0x32, 0xC0, 0xB1, 0xC2, 0xD7, 0x4C, 0xB6, 0xF4, 0x47, 0x29, 0x01, 0x81};
unsigned char FWprime2[32] = {0xCA, 0xD6, 0x2D, 0x76, 0x5F, 0xAA, 0x1C, 0xF3, 0xBD, 0xE4, 0x1B, 0x69, 0x77, 0xAD, 0xC6, 0xCD, 0x55, 0xB6, 0x66, 0xFD, 0xC0, 0xE4, 0x30, 0xA8, 0x08, 0x96, 0xD2, 0x39, 0x5A, 0x86, 0xD3, 0xA5};
unsigned char FWprimeExponent1[32] = {0xDF, 0x80, 0x71, 0x39, 0x4F, 0xDA, 0x9E, 0x43, 0x96, 0x66, 0x70, 0x36, 0xDF, 0xA2, 0xE8, 0x56, 0xB6, 0xF4, 0xD1, 0x82, 0xA3, 0xA8, 0xEA, 0x10, 0xD5, 0x09, 0xD6, 0x4B, 0xA4, 0x2D, 0x05, 0x81}; 
unsigned char FWprimeExponent2[32] = {0x95, 0x02, 0x2C, 0x99, 0xC6, 0x83, 0x81, 0x0B, 0x6A, 0x97, 0xB5, 0xAF, 0x90, 0x35, 0x00, 0x53, 0xFE, 0x40, 0xA6, 0x7E, 0x8A, 0x02, 0xD8, 0xDF, 0xC8, 0x97, 0x8B, 0xCE, 0x88, 0x7B, 0x7D, 0xE5};
unsigned char FWcoefficient[32] = {0x1E, 0xD4, 0xBD, 0x79, 0x02, 0x8C, 0x23, 0xC1, 0x5E, 0x64, 0x12, 0x80, 0x01, 0xC6, 0xCA, 0xCD, 0x4B, 0x81, 0x51, 0x3D, 0x93, 0x45, 0xD2, 0x58, 0x21, 0x68, 0x00, 0xD1, 0x31, 0x91, 0x8D, 0x85};   


unsigned char PublicKey[64] = {0xB9, 0x88, 0x05, 0x50, 0x07, 0x6C, 0x5A, 0x74, 0x07, 0xB9, 0x60, 0x57, 0x9A, 0xB4, 0x0F, 0xE0, 0xED, 0xAA, 0xE4, 0x62, 0xD1, 0xA3, 0x02, 0xCE, 0xAD, 0x96, 0xCD, 0x57, 0xCB, 0x78, 0xD9, 0x18, 0xF2, 0xC1, 0x57, 0x3C, 0xFB, 0x56, 0xFB, 0xCD, 0x90, 0xFC, 0x24, 0x3D, 0xA7, 0xB5, 0xE8, 0x14, 0xC8, 0x16, 0xF9, 0xC3, 0x5E, 0x06, 0xC9, 0x2A, 0xD0, 0xD4, 0x70, 0xCE, 0x0F, 0xF4, 0xFC, 0xCD};


void GetRadomBuff(unsigned char *buff, int cnt)
{
    vu32 ID1;
    int i;
    ID1 = *(vu32*)(ID1_ADDR);
    for(i = 0; i < cnt / 4; i++)
    {
        buff[i * 4] = buff[i * 4] ^ (ID1 >> 24);
        buff[i * 4 + 1] = buff[i * 4 + 1] ^ (ID1 >> 16);
        buff[i * 4 + 2] = buff[i * 4 + 2] ^ (ID1 >> 8);
        buff[i * 4 + 3] = buff[i * 4 + 3] ^ ID1;
    }
}


//RSA密钥存放的空间在1K以内
void MixRsaBuff(unsigned char *buff, int cnt, int StartOffset)
{
    vu32 ID2;
    int i;
    ID2 = *(vu32*)(ID2_ADDR);

    for(i = 0; i < cnt / 4; i++)
    {
        buff[i * 4] = buff[i * 4] ^ (ID2 >> 24);
        buff[i * 4 + 1] = buff[i * 4 + 1] ^ (ID2 >> 16);
        buff[i * 4 + 2] = buff[i * 4 + 2] ^ (ID2 >> 8);
        buff[i * 4 + 3] = buff[i * 4 + 3] ^ ID2;
    }

    for(i = 0; i < cnt; i++)
    {
        WriteBuff[StartOffset + i] = buff[i];
    }

	IWDG_ReloadCounter();
}



void MixRsaKey()
{
    MixRsaBuff(modulus, 64, OFFSET_MODULUS);
    MixRsaBuff(publicExponent, 64, OFFSET_PUBLIC_EXPONENT);
    MixRsaBuff(exponent, 64, OFFSET_EXPONENT);
    MixRsaBuff(prime1, 32, OFFSET_PRIME0);
    MixRsaBuff(prime2, 32, OFFSET_PRIME1);
    MixRsaBuff(primeExponent1, 32, OFFSET_PRIME_EXPONENT0);
    MixRsaBuff(primeExponent2, 32, OFFSET_PRIME_EXPONENT1);
    MixRsaBuff(coefficient, 32, OFFSET_COEFFICIENT);
}

void MixFwRsaKey()
{
    MixRsaBuff(FWmodulus, 64, OFFSET_FW_MODULUS);
    MixRsaBuff(FWpublicExponent, 64, OFFSET_FW_PUBLIC_EXPONENT);
    MixRsaBuff(FWexponent, 64, OFFSET_FW_EXPONENT);
    MixRsaBuff(FWprime1, 32, OFFSET_FW_PRIME0);
    MixRsaBuff(FWprime2, 32, OFFSET_FW_PRIME1);
    MixRsaBuff(FWprimeExponent1, 32, OFFSET_FW_PRIME_EXPONENT0);
    MixRsaBuff(FWprimeExponent2, 32, OFFSET_FW_PRIME_EXPONENT1);
    MixRsaBuff(FWcoefficient, 32, OFFSET_FW_COEFFICIENT);
}

void MixAnotherPublicKey()
{
    MixRsaBuff(PublicKey, 64, 888);
    //此处的publicExponent同私钥的publicExponent，故而不写入
}


void MixChipId()
{
    unsigned char MyWish[] = {"I hope ererything goes well."};//28
    vu32 ID0, ID2, ID1;
    vu8 *ptr;
	vu32 *PtrTemp;
    vu32 i, j;
	vu32 DataTemp;
    u32 temp;
    unsigned char FlashId[12] = {"just  change"};
    ID0 = *(vu32*)(ID0_ADDR);
    ID1 = *(vu32*)(ID1_ADDR);
    ID2 = *(vu32*)(ID2_ADDR);

    ptr = (u8 *)happy;
    for(i = 0; i < sizeof(happy) / 4; i++)
    {
	    DataTemp = *ptr++;
	    DataTemp |= (*ptr++ << 8);
		DataTemp |= (*ptr++ << 16);
		DataTemp |= (*ptr++ << 24);
        ID0 ^= DataTemp;
    }

    ptr = (u8 *)free;
    for(i = 0; i < sizeof(free) / 4; i++)
    {
	    DataTemp = *ptr++;
	    DataTemp |= (*ptr++ << 8);
		DataTemp |= (*ptr++ << 16);
		DataTemp |= (*ptr++ << 24);
        ID1 ^= DataTemp;
    }

    ptr = (u8 *)MyWish;
    for(i = 0; i < sizeof(MyWish) / 4; i++)
    {
	    DataTemp = *ptr++;
	    DataTemp |= (*ptr++ << 8);
		DataTemp |= (*ptr++ << 16);
		DataTemp |= (*ptr++ << 24);
        ID2 ^= DataTemp;
    }

    temp = ID2;
    PtrTemp = (u32 *)FlashId;
    ID2 = ID1 - *PtrTemp++;
    ID1 = ID0 + *PtrTemp++;
    ID0 = temp ^ *PtrTemp;

    WriteBuff[OFFSET_SECRET] = (u8)ID2;
    WriteBuff[OFFSET_SECRET + 1] = (u8)(ID2 >> 8);
    WriteBuff[OFFSET_SECRET + 2] = (u8)(ID2 >> 16);
    WriteBuff[OFFSET_SECRET + 3] = (u8)(ID2 >> 24);

    WriteBuff[OFFSET_SECRET + 4] = (u8)ID1;
    WriteBuff[OFFSET_SECRET + 5] = (u8)(ID1 >> 8);
    WriteBuff[OFFSET_SECRET + 6] = (u8)(ID1 >> 16);
    WriteBuff[OFFSET_SECRET + 7] = (u8)(ID1 >> 24);

    WriteBuff[OFFSET_SECRET + 8] = (u8)ID0;
    WriteBuff[OFFSET_SECRET + 9] = (u8)(ID0 >> 8);
    WriteBuff[OFFSET_SECRET + 10] = (u8)(ID0 >> 16);
    WriteBuff[OFFSET_SECRET + 11] = (u8)(ID0 >> 24);
}

int WriteSecretData(int cnt)
{
    int i;
    FLASH_Unlock();

    if(FLASH_ErasePage(SECRET_ADDRESS) != FLASH_COMPLETE)
	  {
	    return 4;
    }

	
    for(i = 0; i < cnt / 4; i++)
    {
        if(FLASH_ProgramWord(SECRET_ADDRESS + i * 4, *(unsigned int *)(WriteBuff + i * 4)) != FLASH_COMPLETE)
		{
	    	return 1;
		}

	 //IWDG_ReloadCounter();
    }
    
    FLASH_Lock();
    return 0;
}

void BkpHwInit()
{
    /*RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
    //APP启动后，就把BKP的值写为0，表示进入过APP了
    BKP_WriteBackupRegister(BKP_DR1, 0);*/
}



		   
void mDelay(int ms)
{
    volatile int i, j;
	

    for(i = 0; i < ms; i++)
	  {
			//IWDG_ReloadCounter();
        for(j = 0; j < 5000; j++);
		}
}

int CheckWriteData(int cnt)
{
    int i;
	//int j;
	unsigned char *DataPtr = (unsigned char *)SECRET_ADDRESS;
	

	for(i = 0; i < cnt; i++)
	{
	   if(WriteBuff[i] != *DataPtr)
	   {
	       /*for(j = 0; j < 10; j++)
		   {
		      ON_CHIP_LED_OFF;
			  mDelay(500);
			  ON_CHIP_LED_ON;
			  mDelay(500);
			   
		   }*/
	       return 1;
	   }
	   DataPtr++;
	}

	return 0;
}

//TEA密钥采用隔一个写入的方式进行存储，中间参杂一个随机数
//其占用32字节的空间
void MixTeaKey(int StartOffset)
{
    unsigned char TeaKey[16] = {0x41, 0x69, 0xf4, 0xa5, 
                                                 0x8b, 0x91, 0x5c, 0x7d, 
                                                 0x02, 0x5b, 0xc4, 0x12, 
                                                 0xdc, 0x52, 0x27, 0x39};
    vu32 ID0;          
    int i;
    ID0 = *(vu32*)(ID0_ADDR);
    
    for(i = 0; i < 16 / 4; i++)
    {
        WriteBuff[StartOffset + i * 8] = TeaKey[i * 4] ^ (ID0 >> 24);
        WriteBuff[StartOffset + i * 8 + 2] = TeaKey[i * 4 + 1] ^ (ID0 >> 16);
        WriteBuff[StartOffset + i * 8 + 4] = TeaKey[i * 4 + 2] ^ (ID0 >> 8);
        WriteBuff[StartOffset + i * 8 + 6] = TeaKey[i * 4 + 3] ^ ID0;
    }
    

    
}

int main(void)
{								 	
#ifdef DEBUG
  debug();
#endif
  int ret;
  //int mod;
  int ReWriteCnt = 0;
  int CheckRet;

  /* System clocks configuration ---------------------------------------------*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, DISABLE);

  //IWDG_ReloadCounter();


  GetRadomBuff(WriteBuff, FLASH_PAGE_SIZE);
  //IWDG_ReloadCounter();



  MixRsaKey();
	
	MixFwRsaKey();
  //IWDG_ReloadCounter();

  MixAnotherPublicKey();
  //IWDG_ReloadCounter();


  MixChipId();
  MixTeaKey(1900);
  //IWDG_ReloadCounter();


  ReWriteCnt = 0;
  do
  {
      ret = WriteSecretData(FLASH_PAGE_SIZE);
	  CheckRet = CheckWriteData(FLASH_PAGE_SIZE);
      //mod = ReWriteCnt % 4;
	  //ON_CHIP_LED_OFF;
	  //OneOnChipLedOnOffControl(mod, 1);
	  

	  ReWriteCnt++;
	  //IWDG_ReloadCounter();
	  mDelay(250);
  }while((ret != 0 || CheckRet != 0) && ReWriteCnt < 100);
  //IWDG_ReloadCounter();


  

			
  /*JumpAddress = *(volatile unsigned int*) (0x08000000 + 4);     
  Jump_To_Application = (pFunction) JumpAddress;//Jump to user application       
  __set_MSP(*(volatile unsigned int*) 0x08000000);//Initialize user application's Stack Pointer 
  Jump_To_Application();   */

  while (1)				   
  {
  }
}


#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE****/
