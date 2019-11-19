#ifndef __KEY__
#define __KEY__


#define GET_KEY_MINUS		GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_10)
#define GET_KEY_PLUS		GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_9)



//系统中按键的类型
typedef enum {NO_KEY = 0, LEFT_CLICK = 1, LEFT_LONG_PRESS = 2, 
                         RIGHT_CLICK = 3, RIGHT_LONG_PRESS = 4} MenuKeyType;


void KeyHwInit(void);
void KeyStatusHandler(void);
MenuKeyType GetMenuKey(void);
void KeyTopLevelHandler(void);

void SlientKeyHwInit(void);

void SlientKeyHandler(void);


#endif


