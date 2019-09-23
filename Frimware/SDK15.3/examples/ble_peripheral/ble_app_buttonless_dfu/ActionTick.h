#ifndef __ACTION_TICK__
#define __ACTION_TICK__

#include "common.h"
#define TIMER_MULTI         10

#define MS(x)			((TIMER_MULTI) * (x))

uint32_t GetSystemTime(void);
void mDelay(int32_t ms);
void SystickConfig(void);

#endif
