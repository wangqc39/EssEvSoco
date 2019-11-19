#ifndef __ACTIONTICK__
#define __ACTIONTICK__


#define TIMER_MULTI                 1

void SystemTickInterruptHandler(void);
void ActionTickHwInit(void);
u32 GetSystemTime(void);
void mDelay(int ms);
void uDelay(int us);
#endif

