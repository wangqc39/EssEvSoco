#ifndef __ON_CHIP_LED__
#define __ON_CHIP_LED__

#define ON_CHIP_LED_OFF			GPIO_SetBits(GPIOC, GPIO_Pin_3);
#define ON_CHIP_LED_ON			GPIO_ResetBits(GPIOC, GPIO_Pin_3);

void OnChipLedHwInit(void);
void OnChipLedHandler(void);

#endif


