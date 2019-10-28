#ifndef __ACTION_TICK__
#define __ACTION_TICK__

#define TIMER_MULTI		10

unsigned int GetSystemTime(void);
void mDelay(int ms);
void SystemTickConfiguration(void);
void SystemTickInterruptHandler(void);

#endif

