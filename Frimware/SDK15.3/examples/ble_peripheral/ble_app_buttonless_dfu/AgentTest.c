//#include "common.h"
//#include "key.h"
//#include "ActionTick.h"


//FunctionalState AgingTestFlag;

//int SetAgingTestBySystemConfig(FunctionalState *flag)
//{
//    if(*flag > 1)
//    {
//        *flag = DISABLE;
//        AgingTestFlag = *flag;
//        return -1;
//    }
//    AgingTestFlag = *flag;
//    return 0;
//}

//int16_t AgingTestSpeedHandler()
//{
//    static int16_t DistSpeed = 4900;
//    static uint32_t LeftDownTime;
//    static uint32_t RightDownTime;
//    if(GET_KEY_MINUS == 0)
//    {
//        //左键按下
//        if(GetSystemTime() - LeftDownTime > 100)
//        {
//            DistSpeed -= 10;
//            LeftDownTime = GetSystemTime();
//        }
//    }
//    else if(GET_KEY_PLUS == 0)
//    {
//        //右键按下
//        if(GetSystemTime() - RightDownTime > 100)
//        {
//            DistSpeed += 10;
//            RightDownTime = GetSystemTime();
//        }
//    }
//    else
//    {
//        
//    }

//    return DistSpeed;
//    
//}


