#ifndef __APPLICATION_RELATIONSHIP__
#define __APPLICATION_RELATIONSHIP__

//#define ApplicationAddress    0x8003000
//此处的pagesize为一次擦除的大小
#define STM32_PAGE_SIZE                         (0x800)//8B系列
//#define STM32_PAGE_SIZE                         (0x800)//CDE系列
//#define STM32_FLASH_SIZE			(0x40000) /* 512K */
#define STM32_FLASH_SIZE				(256 * 1024)
#define STM32_PAGE_CNT		(STM32_FLASH_SIZE / STM32_PAGE_SIZE)

//#define FLASH_BLOCK_CNT	8192
//#define FLASH_BLOCK_SIZE	512

void RunToApplication(unsigned int Addr);
unsigned int OnChipFlashPagesMask(volatile unsigned int Size);
#endif


