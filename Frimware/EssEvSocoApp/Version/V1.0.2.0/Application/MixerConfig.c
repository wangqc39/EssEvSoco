#include "Common.h"
#include <string.h>
#include "MixerConfig.h"
#include "SpiFlash.h"
#include "MotorSpeed.h"
#include "engine.h"
#include "mixer.h"
#include "Param.h"
#include "MotorSpeedHal.h"

uint32_t Resverd;


#define OFFSET_VEHICLE_PARAM	0

#define PARAM_NAME_1			engine.EngineAccResponseP
#define PARAM_DAFAULT_1			10000
#define PARAM_OFFSET_1			OFFSET_VEHICLE_PARAM
#define PARAM_LENGTH_1			2
#define PARAM_MIN_1				10
#define PARAM_MAX_1				60000

#define PARAM_NAME_2			engine.EngineDecResponse
#define PARAM_DAFAULT_2			10000
#define PARAM_OFFSET_2			(PARAM_OFFSET_1 + PARAM_LENGTH_1)
#define PARAM_LENGTH_2			2
#define PARAM_MIN_2				10
#define PARAM_MAX_2				61600

#define PARAM_NAME_3			engine.FlameOutSwitch
#define PARAM_DAFAULT_3			(ENABLE)
#define PARAM_OFFSET_3			(PARAM_OFFSET_2 + PARAM_LENGTH_2)
#define PARAM_LENGTH_3			1
#define PARAM_MIN_3				(DISABLE)
#define PARAM_MAX_3				(ENABLE)
	
#define PARAM_NAME_4			engine.FlameOutTime
#define PARAM_DAFAULT_4			(5000)
#define PARAM_OFFSET_4 			(PARAM_OFFSET_3 + PARAM_LENGTH_3)
#define PARAM_LENGTH_4			2
#define PARAM_MIN_4				(0)
#define PARAM_MAX_4				(60000)


/*#define PARAM_NAME_5			MotorSpeed.GearBox.GearBoxEnableFlag
#define PARAM_DAFAULT_5			(DISABLE)
#define PARAM_OFFSET_5 			(PARAM_OFFSET_4 + PARAM_LENGTH_4)
#define PARAM_LENGTH_5			1
#define PARAM_MIN_5				(DISABLE)
#define PARAM_MAX_5				(ENABLE)*/
#define PARAM_NAME_5			MotorSpeed.SpeedMode
#define PARAM_DAFAULT_5		(SPEED_MODE_HAL)
#define PARAM_OFFSET_5 			(PARAM_OFFSET_4 + PARAM_LENGTH_4)
#define PARAM_LENGTH_5			1
#define PARAM_MIN_5			(SPEED_MODE_TURN)
#define PARAM_MAX_5			(SPEED_MODE_HAL)

#define PARAM_NAME_6			MotorSpeed.GearBox.MaxGearLevel
#define PARAM_DAFAULT_6			(4)
#define PARAM_OFFSET_6 			(PARAM_OFFSET_5 + PARAM_LENGTH_5)
#define PARAM_LENGTH_6			1
#define PARAM_MIN_6				(1)
#define PARAM_MAX_6				(6)

#define PARAM_NAME_7			MotorSpeed.GearBox.AfterUpGearSpeedPercent
#define PARAM_DAFAULT_7			(58)
#define PARAM_OFFSET_7 			(PARAM_OFFSET_6 + PARAM_LENGTH_6)
#define PARAM_LENGTH_7			1
#define PARAM_MIN_7				(10)
#define PARAM_MAX_7				(90)

#define PARAM_NAME_8			MotorSpeed.GearBox.UpGearSpeedPercent
#define PARAM_DAFAULT_8			(100)
#define PARAM_OFFSET_8 			(PARAM_OFFSET_7 + PARAM_LENGTH_7)
#define PARAM_LENGTH_8			(1)
#define PARAM_MIN_8				(70)
#define PARAM_MAX_8				(100)

#define PARAM_NAME_9			MotorSpeed.GearBox.UpGearDelayTime
#define PARAM_DAFAULT_9			(200)
#define PARAM_OFFSET_9 			(PARAM_OFFSET_8 + PARAM_LENGTH_8)
#define PARAM_LENGTH_9			(2)
#define PARAM_MIN_9				(1)
#define PARAM_MAX_9				(10000)

#define PARAM_NAME_10			MotorSpeed.GearBox.DownGearDelayTime
#define PARAM_DAFAULT_10		(200)
#define PARAM_OFFSET_10 		(PARAM_OFFSET_9 + PARAM_LENGTH_9)
#define PARAM_LENGTH_10			(2)
#define PARAM_MIN_10			(1)
#define PARAM_MAX_10			(10000)

#define PARAM_NAME_11			engine.ThrottleUpResponse
#define PARAM_DAFAULT_11		(50)
#define PARAM_OFFSET_11 		(PARAM_OFFSET_10 + PARAM_LENGTH_10)
#define PARAM_LENGTH_11			(2)
#define PARAM_MIN_11			(4)
#define PARAM_MAX_11			(1000)

#define PARAM_NAME_12			engine.ThrottleDownResponseOri
#define PARAM_DAFAULT_12		(60)
#define PARAM_OFFSET_12 		(PARAM_OFFSET_11 + PARAM_LENGTH_11)
#define PARAM_LENGTH_12			(2)
#define PARAM_MIN_12			(4)
#define PARAM_MAX_12			(1000)

#define PARAM_NAME_13			engine.OverloadExistFlag
#define PARAM_DAFAULT_13		(DISABLE)
#define PARAM_OFFSET_13 		(PARAM_OFFSET_12 + PARAM_LENGTH_12)
#define PARAM_LENGTH_13			(1)
#define PARAM_MIN_13			(DISABLE)
#define PARAM_MAX_13			(ENABLE)

#define PARAM_NAME_14			engine.OverloadEnableFlag
#define PARAM_DAFAULT_14		(DISABLE)
#define PARAM_OFFSET_14 		(PARAM_OFFSET_13 + PARAM_LENGTH_13)
#define PARAM_LENGTH_14			(1)
#define PARAM_MIN_14			(DISABLE)
#define PARAM_MAX_14			(ENABLE)

#define PARAM_NAME_15			Resverd//HalMaxSpeedCalibrate//engine.OverloadDelayTime
#define PARAM_DAFAULT_15		(90)
#define PARAM_OFFSET_15 		(PARAM_OFFSET_14 + PARAM_LENGTH_14)
#define PARAM_LENGTH_15			(2)
#define PARAM_MIN_15			(60)
#define PARAM_MAX_15			(60000)

#define PARAM_NAME_16			engine.AccelerateArrayOri[0]
#define PARAM_DAFAULT_16		(300)
#define PARAM_OFFSET_16 		(PARAM_OFFSET_15 + PARAM_LENGTH_15)
#define PARAM_LENGTH_16			(2)
#define PARAM_MIN_16			(1)
#define PARAM_MAX_16			(3000)

#define PARAM_NAME_17			engine.AccelerateArrayOri[1]
#define PARAM_DAFAULT_17		(400)
#define PARAM_OFFSET_17 		(PARAM_OFFSET_16 + PARAM_LENGTH_16)
#define PARAM_LENGTH_17			(2)
#define PARAM_MIN_17			(1)
#define PARAM_MAX_17			(3000)

#define PARAM_NAME_18			engine.AccelerateArrayOri[2]
#define PARAM_DAFAULT_18		(500)
#define PARAM_OFFSET_18 		(PARAM_OFFSET_17 + PARAM_LENGTH_17)
#define PARAM_LENGTH_18			(2)
#define PARAM_MIN_18			(1)
#define PARAM_MAX_18			(3000)

#define PARAM_NAME_19			engine.AccelerateArrayOri[3]
#define PARAM_DAFAULT_19		(600)
#define PARAM_OFFSET_19 		(PARAM_OFFSET_18 + PARAM_LENGTH_18)
#define PARAM_LENGTH_19			(2)
#define PARAM_MIN_19			(1)
#define PARAM_MAX_19			(3000)

#define PARAM_NAME_20			engine.AccelerateArrayOri[4]
#define PARAM_DAFAULT_20		(700)
#define PARAM_OFFSET_20 		(PARAM_OFFSET_19 + PARAM_LENGTH_19)
#define PARAM_LENGTH_20			(2)
#define PARAM_MIN_20			(1)
#define PARAM_MAX_20			(3000)

#define PARAM_NAME_21			engine.AccelerateArrayOri[5]
#define PARAM_DAFAULT_21		(800)
#define PARAM_OFFSET_21			(PARAM_OFFSET_20 + PARAM_LENGTH_20)
#define PARAM_LENGTH_21			(2)
#define PARAM_MIN_21			(1)
#define PARAM_MAX_21			(3000)

#define PARAM_NAME_22			engine.AccelerateArrayOri[6]
#define PARAM_DAFAULT_22		(900)
#define PARAM_OFFSET_22 		(PARAM_OFFSET_21 + PARAM_LENGTH_21)
#define PARAM_LENGTH_22			(2)
#define PARAM_MIN_22			(1)
#define PARAM_MAX_22			(3000)

#define PARAM_NAME_23			engine.AccelerateArrayOri[7]
#define PARAM_DAFAULT_23		(1000)
#define PARAM_OFFSET_23 		(PARAM_OFFSET_22 + PARAM_LENGTH_22)
#define PARAM_LENGTH_23			(2)
#define PARAM_MIN_23			(1)
#define PARAM_MAX_23			(3000)

#define PARAM_NAME_24			engine.AccelerateArrayOri[8]
#define PARAM_DAFAULT_24		(1100)
#define PARAM_OFFSET_24 		(PARAM_OFFSET_23 + PARAM_LENGTH_23)
#define PARAM_LENGTH_24			(2)
#define PARAM_MIN_24			(1)
#define PARAM_MAX_24			(3000)

#define PARAM_NAME_25			engine.AccelerateArrayOri[9]
#define PARAM_DAFAULT_25		(1200)
#define PARAM_OFFSET_25 		(PARAM_OFFSET_24 + PARAM_LENGTH_24)
#define PARAM_LENGTH_25			(2)
#define PARAM_MIN_25			(1)
#define PARAM_MAX_25			(3000)

#define PARAM_NAME_26			engine.AccelerateArrayOri[10]
#define PARAM_DAFAULT_26		(1300)
#define PARAM_OFFSET_26 		(PARAM_OFFSET_25 + PARAM_LENGTH_25)
#define PARAM_LENGTH_26			(2)
#define PARAM_MIN_26			(1)
#define PARAM_MAX_26			(3000)

#define PARAM_NAME_27			engine.AccelerateArrayOri[11]
#define PARAM_DAFAULT_27		(1400)
#define PARAM_OFFSET_27 		(PARAM_OFFSET_26 + PARAM_LENGTH_26)
#define PARAM_LENGTH_27			(2)
#define PARAM_MIN_27			(1)
#define PARAM_MAX_27			(3000)

#define PARAM_NAME_28			engine.AccelerateArrayOri[12]
#define PARAM_DAFAULT_28		(1400)
#define PARAM_OFFSET_28 		(PARAM_OFFSET_27 + PARAM_LENGTH_27)
#define PARAM_LENGTH_28			(2)
#define PARAM_MIN_28			(1)
#define PARAM_MAX_28			(3000)

#define PARAM_NAME_29			engine.AccelerateArrayOri[13]
#define PARAM_DAFAULT_29		(1400)
#define PARAM_OFFSET_29 		(PARAM_OFFSET_28 + PARAM_LENGTH_28)
#define PARAM_LENGTH_29			(2)
#define PARAM_MIN_29			(1)
#define PARAM_MAX_29			(3000)

#define PARAM_NAME_30			engine.AccelerateArrayOri[14]
#define PARAM_DAFAULT_30		(1400)
#define PARAM_OFFSET_30 		(PARAM_OFFSET_29 + PARAM_LENGTH_29)
#define PARAM_LENGTH_30			(2)
#define PARAM_MIN_30			(1)
#define PARAM_MAX_30			(3000)

#define PARAM_NAME_31			engine.AccelerateArrayOri[15]
#define PARAM_DAFAULT_31		(1400)
#define PARAM_OFFSET_31 		(PARAM_OFFSET_30 + PARAM_LENGTH_30)
#define PARAM_LENGTH_31			(2)
#define PARAM_MIN_31			(1)
#define PARAM_MAX_31			(3000)

#define PARAM_NAME_32			engine.AccelerateArrayOri[16]
#define PARAM_DAFAULT_32		(1400)
#define PARAM_OFFSET_32 		(PARAM_OFFSET_31 + PARAM_LENGTH_31)
#define PARAM_LENGTH_32			(2)
#define PARAM_MIN_32			(1)
#define PARAM_MAX_32			(3000)

#define PARAM_NAME_33			engine.AccelerateArrayOri[17]
#define PARAM_DAFAULT_33		(1400)
#define PARAM_OFFSET_33 		(PARAM_OFFSET_32 + PARAM_LENGTH_32)
#define PARAM_LENGTH_33			(2)
#define PARAM_MIN_33			(1)
#define PARAM_MAX_33			(3000)

#define PARAM_NAME_34			engine.AccelerateArrayOri[18]
#define PARAM_DAFAULT_34		(1400)
#define PARAM_OFFSET_34 		(PARAM_OFFSET_33 + PARAM_LENGTH_33)
#define PARAM_LENGTH_34			(2)
#define PARAM_MIN_34			(1)
#define PARAM_MAX_34			(3000)

#define PARAM_NAME_35			engine.AccelerateArrayOri[19]
#define PARAM_DAFAULT_35		(1400)
#define PARAM_OFFSET_35 		(PARAM_OFFSET_34 + PARAM_LENGTH_34)
#define PARAM_LENGTH_35			(2)
#define PARAM_MIN_35			(1)
#define PARAM_MAX_35			(3000)

#define PARAM_NAME_36			engine.EngineAccResponseD
#define PARAM_DAFAULT_36		10000
#define PARAM_OFFSET_36			(PARAM_OFFSET_35 + PARAM_LENGTH_35)
#define PARAM_LENGTH_36			2
#define PARAM_MIN_36			10
#define PARAM_MAX_36			60000







#define PARAM_NAME(x)			PARAM_NAME_##x
#define PARAM_OFFSET(x)			PARAM_OFFSET_##x
#define PARAM_LENGTH(x)			PARAM_LENGTH_##x
#define PARAM_DEFAULT(x)			PARAM_DAFAULT_##x
#define PARAM_MIN(x)				PARAM_MIN_##x
#define PARAM_MAX(x)				PARAM_MAX_##x

//获取一个系统信息
//返回值为系统信息的内容长度
//如果Index错误，返回-1	
int32_t GetOneVehicleParam(uint8_t *buff, uint8_t Index)
{
    int32_t length;
    uint8_t *SrcPtr;
    switch(Index)
    {
        case 1:
            SrcPtr = (unsigned char *)&PARAM_NAME(1);
            length = PARAM_LENGTH(1);
            break;
        case 2:
            SrcPtr = (unsigned char *)&PARAM_NAME(2);
            length = PARAM_LENGTH(2);
            break;
        case 3:
            SrcPtr = (unsigned char *)&PARAM_NAME(3);
            length = PARAM_LENGTH(3);
            break;
        case 4:
            SrcPtr = (unsigned char *)&PARAM_NAME(4);
            length = PARAM_LENGTH(4);
            break;  
        case 5:
            SrcPtr = (unsigned char *)&PARAM_NAME(5);
            length = PARAM_LENGTH(5);
            break;  
        case 6:
            SrcPtr = (unsigned char *)&PARAM_NAME(6);
            length = PARAM_LENGTH(6);
            break;  
        case 7:
            SrcPtr = (unsigned char *)&PARAM_NAME(7);
            length = PARAM_LENGTH(7);
            break;  
        case 8:
            SrcPtr = (unsigned char *)&PARAM_NAME(8);
            length = PARAM_LENGTH(8);
            break;  
        case 9:
            SrcPtr = (unsigned char *)&PARAM_NAME(9);
            length = PARAM_LENGTH(9);
            break;  
        case 10:
            SrcPtr = (unsigned char *)&PARAM_NAME(10);
            length = PARAM_LENGTH(10);
            break;  
        case 11:
            SrcPtr = (unsigned char *)&PARAM_NAME(11);
            length = PARAM_LENGTH(11);
            break;  
        case 12:
            SrcPtr = (unsigned char *)&PARAM_NAME(12);
            length = PARAM_LENGTH(12);
            break;  
        case 13:
            SrcPtr = (unsigned char *)&PARAM_NAME(13);
            length = PARAM_LENGTH(13);
            break;  
        case 14:
            SrcPtr = (unsigned char *)&PARAM_NAME(14);
            length = PARAM_LENGTH(14);
            break;  
        case 15:
            SrcPtr = (unsigned char *)&PARAM_NAME(15);
            length = PARAM_LENGTH(15);
            break;  
        case 16:
            SrcPtr = (unsigned char *)&PARAM_NAME(16);
            length = PARAM_LENGTH(16);
            break;  
        case 17:
            SrcPtr = (unsigned char *)&PARAM_NAME(17);
            length = PARAM_LENGTH(17);
            break;  
        case 18:
            SrcPtr = (unsigned char *)&PARAM_NAME(18);
            length = PARAM_LENGTH(18);
            break;  
        case 19:
            SrcPtr = (unsigned char *)&PARAM_NAME(19);
            length = PARAM_LENGTH(19);
            break;  
        case 20:
            SrcPtr = (unsigned char *)&PARAM_NAME(20);
            length = PARAM_LENGTH(20);
            break;  
        case 21:
            SrcPtr = (unsigned char *)&PARAM_NAME(21);
            length = PARAM_LENGTH(21);
            break;  
        case 22:
            SrcPtr = (unsigned char *)&PARAM_NAME(22);
            length = PARAM_LENGTH(22);
            break;  
        case 23:
            SrcPtr = (unsigned char *)&PARAM_NAME(23);
            length = PARAM_LENGTH(23);
            break;  
        case 24:
            SrcPtr = (unsigned char *)&PARAM_NAME(24);
            length = PARAM_LENGTH(24);
            break;  
        case 25:
            SrcPtr = (unsigned char *)&PARAM_NAME(25);
            length = PARAM_LENGTH(25);
            break;  
        case 26:
            SrcPtr = (unsigned char *)&PARAM_NAME(26);
            length = PARAM_LENGTH(26);
            break;  
        case 27:
            SrcPtr = (unsigned char *)&PARAM_NAME(27);
            length = PARAM_LENGTH(27);
            break;  
        case 28:
            SrcPtr = (unsigned char *)&PARAM_NAME(28);
            length = PARAM_LENGTH(28);
            break;  
        case 29:
            SrcPtr = (unsigned char *)&PARAM_NAME(29);
            length = PARAM_LENGTH(29);
            break;  
        case 30:
            SrcPtr = (unsigned char *)&PARAM_NAME(30);
            length = PARAM_LENGTH(30);
            break;  
        case 31:
            SrcPtr = (unsigned char *)&PARAM_NAME(31);
            length = PARAM_LENGTH(31);
            break;  
        case 32:
            SrcPtr = (unsigned char *)&PARAM_NAME(32);
            length = PARAM_LENGTH(32);
            break;  
        case 33:
            SrcPtr = (unsigned char *)&PARAM_NAME(33);
            length = PARAM_LENGTH(33);
            break;  
        case 34:
            SrcPtr = (unsigned char *)&PARAM_NAME(34);
            length = PARAM_LENGTH(34);
            break;  
        case 35:
            SrcPtr = (unsigned char *)&PARAM_NAME(35);
            length = PARAM_LENGTH(35);
            break;  
        case 36:
            SrcPtr = (unsigned char *)&PARAM_NAME(36);
            length = PARAM_LENGTH(36);
            break;  
      
        default:
            return -1;
    }
    memcpy(buff, SrcPtr, length);
    return length;
}




/*****************************参数设定，直接修改内存中的参数变量*****************************************/
//参数设定后，对于部分存在关联的参数要进行计算以触发其功能
int32_t ActiveVehicleParam(uint8_t index)
{
    switch(index)
    {
        case 2:
        case 12:
            CalDecelerartion();
            break;
        case 14:
            //todo:过载使能修改后的处理
            break;
        case 7:
        case 8:
            GearBoxSpeedReset();
            break;
        default:
            break;
    }
    
    return 0;
}

//先进行参数的检查，如果长度或内容不正常，则返回参数实际值
//如果设定参数正常，则改写内存中变量的参数值
//SetValuePtr:设定值的内存指针
//ParamValuePtr:设定后参数值的指针
//length:设定参数的长度
//Index:参数的索引
int32_t SetOneVehicleParam(uint8_t *SetValuePtr, uint8_t *ParamValuePtr, uint8_t length, uint8_t Index)
{
    uint8_t *ParamAddr;
    int32_t ParamLength;
    switch(Index)
    {
        case 1:
            ParamAddr = (uint8_t *)&PARAM_NAME(1);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(1), length, PARAM_NAME(1), PARAM_MIN(1), PARAM_MAX(1));
            break;
        case 2:
            ParamAddr = (uint8_t *)&PARAM_NAME(2);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(2), length, PARAM_NAME(2), PARAM_MIN(2), PARAM_MAX(2));
            break;
        case 3:
            ParamAddr = (uint8_t *)&PARAM_NAME(3);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(3), length, PARAM_NAME(3), PARAM_MIN(3), PARAM_MAX(3));
            break;
        case 4:
            ParamAddr = (uint8_t *)&PARAM_NAME(4);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(4), length, PARAM_NAME(4), PARAM_MIN(4), PARAM_MAX(4));
            break;
        case 5:
            ParamAddr = (uint8_t *)&PARAM_NAME(5);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(5), length, PARAM_NAME(5), PARAM_MIN(5), PARAM_MAX(5));
            break;
        case 6:
            ParamAddr = (uint8_t *)&PARAM_NAME(6);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(6), length, PARAM_NAME(6), PARAM_MIN(6), PARAM_MAX(6));
            break;
        case 7:
            ParamAddr = (uint8_t *)&PARAM_NAME(7);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(7), length, PARAM_NAME(7), PARAM_MIN(7), PARAM_MAX(7));
            break;
        case 8:
            ParamAddr = (uint8_t *)&PARAM_NAME(8);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(8), length, PARAM_NAME(8), PARAM_MIN(8), PARAM_MAX(8));
            break;
        case 9:
            ParamAddr = (uint8_t *)&PARAM_NAME(9);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(9), length, PARAM_NAME(9), PARAM_MIN(9), PARAM_MAX(9));
            break;
        case 10:
            ParamAddr = (uint8_t *)&PARAM_NAME(10);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(10), length, PARAM_NAME(10), PARAM_MIN(10), PARAM_MAX(10));
            break;
        case 11:
            ParamAddr = (uint8_t *)&PARAM_NAME(11);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(11), length, PARAM_NAME(11), PARAM_MIN(11), PARAM_MAX(11));
            break;
        case 12:
            ParamAddr = (uint8_t *)&PARAM_NAME(12);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(12), length, PARAM_NAME(12), PARAM_MIN(12), PARAM_MAX(12));
            break;
/*
        case 13:
            ParamAddr = (uint8_t *)&PARAM_NAME(13);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(13), length, PARAM_NAME(13), PARAM_MIN(13), PARAM_MAX(13));
            break;
*/
        case 14:
            ParamAddr = (uint8_t *)&PARAM_NAME(14);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(14), length, PARAM_NAME(14), PARAM_MIN(14), PARAM_MAX(14));
            break;
        case 15:
            ParamAddr = (uint8_t *)&PARAM_NAME(15);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(15), length, PARAM_NAME(15), PARAM_MIN(15), PARAM_MAX(15));
            break;
        case 16:
            ParamAddr = (uint8_t *)&PARAM_NAME(16);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(16), length, PARAM_NAME(16), PARAM_MIN(16), PARAM_MAX(16));
            break;
        case 17:
            ParamAddr = (uint8_t *)&PARAM_NAME(17);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(17), length, PARAM_NAME(17), PARAM_MIN(17), PARAM_MAX(17));
            break;
        case 18:
            ParamAddr = (uint8_t *)&PARAM_NAME(18);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(18), length, PARAM_NAME(18), PARAM_MIN(18), PARAM_MAX(18));
            break;
        case 19:
            ParamAddr = (uint8_t *)&PARAM_NAME(19);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(19), length, PARAM_NAME(19), PARAM_MIN(19), PARAM_MAX(19));
            break;
        case 20:
            ParamAddr = (uint8_t *)&PARAM_NAME(20);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(20), length, PARAM_NAME(20), PARAM_MIN(20), PARAM_MAX(20));
            break;
        case 21:
            ParamAddr = (uint8_t *)&PARAM_NAME(21);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(21), length, PARAM_NAME(21), PARAM_MIN(21), PARAM_MAX(21));
            break;
        case 22:
            ParamAddr = (uint8_t *)&PARAM_NAME(22);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(22), length, PARAM_NAME(22), PARAM_MIN(22), PARAM_MAX(22));
            break;
        case 23:
            ParamAddr = (uint8_t *)&PARAM_NAME(23);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(23), length, PARAM_NAME(23), PARAM_MIN(23), PARAM_MAX(23));
            break;
        case 24:
            ParamAddr = (uint8_t *)&PARAM_NAME(24);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(24), length, PARAM_NAME(24), PARAM_MIN(24), PARAM_MAX(24));
            break;
        case 25:
            ParamAddr = (uint8_t *)&PARAM_NAME(25);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(25), length, PARAM_NAME(25), PARAM_MIN(25), PARAM_MAX(25));
            break;
        case 26:
            ParamAddr = (uint8_t *)&PARAM_NAME(26);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(26), length, PARAM_NAME(26), PARAM_MIN(26), PARAM_MAX(26));
            break;
        case 27:
            ParamAddr = (uint8_t *)&PARAM_NAME(27);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(27), length, PARAM_NAME(27), PARAM_MIN(27), PARAM_MAX(27));
            break;
        case 28:
            ParamAddr = (uint8_t *)&PARAM_NAME(28);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(28), length, PARAM_NAME(28), PARAM_MIN(28), PARAM_MAX(28));
            break;
        case 29:
            ParamAddr = (uint8_t *)&PARAM_NAME(29);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(29), length, PARAM_NAME(29), PARAM_MIN(29), PARAM_MAX(29));
            break;
        case 30:
            ParamAddr = (uint8_t *)&PARAM_NAME(30);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(30), length, PARAM_NAME(30), PARAM_MIN(30), PARAM_MAX(30));
            break;
        case 31:
            ParamAddr = (uint8_t *)&PARAM_NAME(31);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(31), length, PARAM_NAME(31), PARAM_MIN(31), PARAM_MAX(31));
            break;
        case 32:
            ParamAddr = (uint8_t *)&PARAM_NAME(32);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(32), length, PARAM_NAME(32), PARAM_MIN(32), PARAM_MAX(32));
            break;
        case 33:
            ParamAddr = (uint8_t *)&PARAM_NAME(33);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(33), length, PARAM_NAME(33), PARAM_MIN(33), PARAM_MAX(33));
            break;
        case 34:
            ParamAddr = (uint8_t *)&PARAM_NAME(34);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(34), length, PARAM_NAME(34), PARAM_MIN(34), PARAM_MAX(34));
            break;
        case 35:
            ParamAddr = (uint8_t *)&PARAM_NAME(35);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(35), length, PARAM_NAME(35), PARAM_MIN(35), PARAM_MAX(35));
            break;
        case 36:
            ParamAddr = (uint8_t *)&PARAM_NAME(36);
            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(36), length, PARAM_NAME(36), PARAM_MIN(36), PARAM_MAX(36));
            break;
        
        default:
            return -1;
    }

    memcpy(ParamAddr, SetValuePtr, ParamLength);
    memcpy(ParamValuePtr, SetValuePtr, ParamLength);

    ActiveVehicleParam(Index);
    return ParamLength;
}


/***************************写入配置表，将内存中的各个变量重组成整张配置表，写入FLASH中********************************/


#define PARAM_TABLE_SIZE		WINBOND_FLASH_SECTOR_SIZE
int32_t WriteVehicleParamTable(uint8_t VehicleIndex)
{
    uint32_t CheckSum;
    __align(4) uint8_t ParamTable[PARAM_TABLE_SIZE];
    
    memset(ParamTable, 0xFF, PARAM_TABLE_SIZE);
     


    WriteOneParamToTable((unsigned char *)&PARAM_NAME(1), PARAM_OFFSET(1), PARAM_LENGTH(1), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(2), PARAM_OFFSET(2), PARAM_LENGTH(2), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(3), PARAM_OFFSET(3), PARAM_LENGTH(3), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(4), PARAM_OFFSET(4), PARAM_LENGTH(4), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(5), PARAM_OFFSET(5), PARAM_LENGTH(5), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(6), PARAM_OFFSET(6), PARAM_LENGTH(6), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(7), PARAM_OFFSET(7), PARAM_LENGTH(7), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(8), PARAM_OFFSET(8), PARAM_LENGTH(8), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(9), PARAM_OFFSET(9), PARAM_LENGTH(9), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(10), PARAM_OFFSET(10), PARAM_LENGTH(10), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(11), PARAM_OFFSET(11), PARAM_LENGTH(11), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(12), PARAM_OFFSET(12), PARAM_LENGTH(12), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(13), PARAM_OFFSET(13), PARAM_LENGTH(13), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(14), PARAM_OFFSET(14), PARAM_LENGTH(14), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(15), PARAM_OFFSET(15), PARAM_LENGTH(15), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(16), PARAM_OFFSET(16), PARAM_LENGTH(16), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(17), PARAM_OFFSET(17), PARAM_LENGTH(17), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(18), PARAM_OFFSET(18), PARAM_LENGTH(18), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(19), PARAM_OFFSET(19), PARAM_LENGTH(19), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(20), PARAM_OFFSET(20), PARAM_LENGTH(20), (unsigned char *)ParamTable);

    WriteOneParamToTable((unsigned char *)&PARAM_NAME(21), PARAM_OFFSET(21), PARAM_LENGTH(21), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(22), PARAM_OFFSET(22), PARAM_LENGTH(22), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(23), PARAM_OFFSET(23), PARAM_LENGTH(23), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(24), PARAM_OFFSET(24), PARAM_LENGTH(24), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(25), PARAM_OFFSET(25), PARAM_LENGTH(25), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(26), PARAM_OFFSET(26), PARAM_LENGTH(26), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(27), PARAM_OFFSET(27), PARAM_LENGTH(27), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(28), PARAM_OFFSET(28), PARAM_LENGTH(28), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(29), PARAM_OFFSET(29), PARAM_LENGTH(29), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(30), PARAM_OFFSET(30), PARAM_LENGTH(30), (unsigned char *)ParamTable);

    WriteOneParamToTable((unsigned char *)&PARAM_NAME(31), PARAM_OFFSET(31), PARAM_LENGTH(31), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(32), PARAM_OFFSET(32), PARAM_LENGTH(32), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(33), PARAM_OFFSET(33), PARAM_LENGTH(33), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(34), PARAM_OFFSET(34), PARAM_LENGTH(34), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(35), PARAM_OFFSET(35), PARAM_LENGTH(35), (unsigned char *)ParamTable);
    WriteOneParamToTable((unsigned char *)&PARAM_NAME(36), PARAM_OFFSET(36), PARAM_LENGTH(36), (unsigned char *)ParamTable);



    CheckSum = CalSum((uint32_t *)ParamTable, (PARAM_TABLE_SIZE >> 2) - 1);
    memcpy(ParamTable + PARAM_TABLE_SIZE - 4, &CheckSum, 4);

    WriteParamSector(ParamTable, SectorAddr(SECTOR_SOUND_CONFIG(VehicleIndex)));
    return 0;
}


void AnalyzeMixerConfig(unsigned char VoiceIndex)
{
    uint8_t MixerConfigBuff[SPI_FLASH_SECTOR_SIZE];
    
    
    DataFlashReadData(SectorAddr(SECTOR_SOUND_CONFIG(VoiceIndex)), MixerConfigBuff, SPI_FLASH_SECTOR_SIZE);
   
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(1), PARAM_OFFSET(1), PARAM_LENGTH(1), PARAM_DEFAULT(1), PARAM_MIN(1), PARAM_MAX(1));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(2), PARAM_OFFSET(2), PARAM_LENGTH(2), PARAM_DEFAULT(2), PARAM_MIN(2), PARAM_MAX(2));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(3), PARAM_OFFSET(3), PARAM_LENGTH(3), PARAM_DEFAULT(3), PARAM_MIN(3), PARAM_MAX(3));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(4), PARAM_OFFSET(4), PARAM_LENGTH(4), PARAM_DEFAULT(4), PARAM_MIN(4), PARAM_MAX(4));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(5), PARAM_OFFSET(5), PARAM_LENGTH(5), PARAM_DEFAULT(5), PARAM_MIN(5), PARAM_MAX(5));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(6), PARAM_OFFSET(6), PARAM_LENGTH(6), PARAM_DEFAULT(6), PARAM_MIN(6), PARAM_MAX(6));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(7), PARAM_OFFSET(7), PARAM_LENGTH(7), PARAM_DEFAULT(7), PARAM_MIN(7), PARAM_MAX(7));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(8), PARAM_OFFSET(8), PARAM_LENGTH(8), PARAM_DEFAULT(8), PARAM_MIN(8), PARAM_MAX(8));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(9), PARAM_OFFSET(9), PARAM_LENGTH(9), PARAM_DEFAULT(9), PARAM_MIN(9), PARAM_MAX(9));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(10), PARAM_OFFSET(10), PARAM_LENGTH(10), PARAM_DEFAULT(10), PARAM_MIN(10), PARAM_MAX(10));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(11), PARAM_OFFSET(11), PARAM_LENGTH(11), PARAM_DEFAULT(11), PARAM_MIN(11), PARAM_MAX(11));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(12), PARAM_OFFSET(12), PARAM_LENGTH(12), PARAM_DEFAULT(12), PARAM_MIN(12), PARAM_MAX(12));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(13), PARAM_OFFSET(13), PARAM_LENGTH(13), PARAM_DEFAULT(13), PARAM_MIN(13), PARAM_MAX(13));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(14), PARAM_OFFSET(14), PARAM_LENGTH(14), PARAM_DEFAULT(14), PARAM_MIN(14), PARAM_MAX(14));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(15), PARAM_OFFSET(15), PARAM_LENGTH(15), PARAM_DEFAULT(15), PARAM_MIN(15), PARAM_MAX(15));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(16), PARAM_OFFSET(16), PARAM_LENGTH(16), PARAM_DEFAULT(16), PARAM_MIN(16), PARAM_MAX(16));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(17), PARAM_OFFSET(17), PARAM_LENGTH(17), PARAM_DEFAULT(17), PARAM_MIN(17), PARAM_MAX(17));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(18), PARAM_OFFSET(18), PARAM_LENGTH(18), PARAM_DEFAULT(18), PARAM_MIN(18), PARAM_MAX(18));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(19), PARAM_OFFSET(19), PARAM_LENGTH(19), PARAM_DEFAULT(19), PARAM_MIN(19), PARAM_MAX(19));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(20), PARAM_OFFSET(20), PARAM_LENGTH(20), PARAM_DEFAULT(20), PARAM_MIN(20), PARAM_MAX(20));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(21), PARAM_OFFSET(21), PARAM_LENGTH(21), PARAM_DEFAULT(21), PARAM_MIN(21), PARAM_MAX(21));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(22), PARAM_OFFSET(22), PARAM_LENGTH(22), PARAM_DEFAULT(22), PARAM_MIN(22), PARAM_MAX(22));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(23), PARAM_OFFSET(23), PARAM_LENGTH(23), PARAM_DEFAULT(23), PARAM_MIN(23), PARAM_MAX(23));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(24), PARAM_OFFSET(24), PARAM_LENGTH(24), PARAM_DEFAULT(24), PARAM_MIN(24), PARAM_MAX(24));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(25), PARAM_OFFSET(25), PARAM_LENGTH(25), PARAM_DEFAULT(25), PARAM_MIN(25), PARAM_MAX(25));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(26), PARAM_OFFSET(26), PARAM_LENGTH(26), PARAM_DEFAULT(26), PARAM_MIN(26), PARAM_MAX(26));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(27), PARAM_OFFSET(27), PARAM_LENGTH(27), PARAM_DEFAULT(27), PARAM_MIN(27), PARAM_MAX(27));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(28), PARAM_OFFSET(28), PARAM_LENGTH(28), PARAM_DEFAULT(28), PARAM_MIN(28), PARAM_MAX(28));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(29), PARAM_OFFSET(29), PARAM_LENGTH(29), PARAM_DEFAULT(29), PARAM_MIN(29), PARAM_MAX(29));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(30), PARAM_OFFSET(30), PARAM_LENGTH(30), PARAM_DEFAULT(30), PARAM_MIN(30), PARAM_MAX(30));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(31), PARAM_OFFSET(31), PARAM_LENGTH(31), PARAM_DEFAULT(31), PARAM_MIN(31), PARAM_MAX(31));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(32), PARAM_OFFSET(32), PARAM_LENGTH(32), PARAM_DEFAULT(32), PARAM_MIN(32), PARAM_MAX(32));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(33), PARAM_OFFSET(33), PARAM_LENGTH(33), PARAM_DEFAULT(33), PARAM_MIN(33), PARAM_MAX(33));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(34), PARAM_OFFSET(34), PARAM_LENGTH(34), PARAM_DEFAULT(34), PARAM_MIN(34), PARAM_MAX(34));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(35), PARAM_OFFSET(35), PARAM_LENGTH(35), PARAM_DEFAULT(35), PARAM_MIN(35), PARAM_MAX(35));
    InitOneParam(MixerConfigBuff, (unsigned char *)&PARAM_NAME(36), PARAM_OFFSET(36), PARAM_LENGTH(36), PARAM_DEFAULT(36), PARAM_MIN(36), PARAM_MAX(36));


    CalDecelerartion();
    CalRealVolume();

}

