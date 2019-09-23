#ifndef __COMMON__
#define __COMMON__
#include <stdint.h>
#include <stdbool.h>

#define NO_BOOTLOADER_DEBUG     1

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;

#define LENGTH_GUID						16



//定义系统中的中断优先级
#define IRQ_PROIRITY_I2S                         6
//#define IRQ_PROIRITY_TIMER_FINAL_MIXER           6
//#define IRQ_PROIRITY_TIMER_MIXER0                6
//#define IRQ_PROIRITY_TIMER_MIXER1                6
#define IRQ_PROIRITY_TIMER_HAL                   6


#define PPI_CHANNEL_ADC_START_TO_SAMPLE             NRF_PPI_CHANNEL1
#define PPI_CHANNEL_ADC_DONE_TO_START               NRF_PPI_CHANNEL0


#define PERCENT_BASE_SHIFT		13
#define PERCENT_BASE				(1 << PERCENT_BASE_SHIFT)





//各种地址
//#define OnChipAudioMenuAddr				0x00030000 //320K
//#define SYSTEM_INFO_ADDR                0x00078000 //临时定义，用于OnChipAudio进行校验，Todo：寻找下一个功能块用到FLASH的地址


#define IWDG_ReloadCounter()            


#endif
