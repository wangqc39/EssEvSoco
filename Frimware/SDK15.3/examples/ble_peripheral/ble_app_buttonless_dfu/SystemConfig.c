#include "common.h"
#include <string.h>
#include "SpiFlash.h"
#include "SystemConfig.h"
#include "SystemError.h"
#include "mixer.h"
#include "SystemInfo.h"
#include "AgentTest.h"
#include "param.h"
#include "MotorSpeed.h"
#include "MotorSpeedHal.h"
#include "Password.h"
#include "Tas5719.h"








#define SECTOR_INDEX_SYSTEM_CONFIG			0


#define OFFSET_SYSTEM_PARAM	0


//设备配置，密码，BIND信息等，仅有内部接口，没有外部接口
#define OFFSET_DEVICE_PARAM 2048

uint32_t Reserved;

#define PARAM_NAME_1			mixer.SoundIndex
#define PARAM_DAFAULT_1			0
#define PARAM_OFFSET_1			OFFSET_SYSTEM_PARAM
#define PARAM_LENGTH_1			1
#define PARAM_MIN_1				0
#define PARAM_MAX_1				5

#define PARAM_NAME_2			mixer.VolumeLevel
#define PARAM_DAFAULT_2			2
#define PARAM_OFFSET_2			(PARAM_OFFSET_1 + PARAM_LENGTH_1)
#define PARAM_LENGTH_2			1
#define PARAM_MIN_2				MIN_VOLUME
#define PARAM_MAX_2				MAX_VOLUME

#define PARAM_NAME_3			MotorSpeed.ThrottleCurveLevel
#define PARAM_DAFAULT_3			5
#define PARAM_OFFSET_3			(PARAM_OFFSET_2 + PARAM_LENGTH_2)
#define PARAM_LENGTH_3			1
#define PARAM_MIN_3				0
#define PARAM_MAX_3				10

#define PARAM_NAME_4			HalCalibrate100KmH//engine.OverloadDelayTime
#define PARAM_DAFAULT_4	    	(89)
#define PARAM_OFFSET_4   		(PARAM_OFFSET_3 + PARAM_LENGTH_3)
#define PARAM_LENGTH_4			(2)
#define PARAM_MIN_4			    (1)
#define PARAM_MAX_4			    (60000)

#define PARAM_NAME_5			MaxVehicleCalibrate//engine.OverloadDelayTime
#define PARAM_DAFAULT_5	    	(100)
#define PARAM_OFFSET_5   		(PARAM_OFFSET_4 + PARAM_LENGTH_4)
#define PARAM_LENGTH_5			(2)
#define PARAM_MIN_5			    (10)
#define PARAM_MAX_5			    (200)


#define PARAM_NAME_6			Reserved//engine.OverloadDelayTime
#define PARAM_DAFAULT_6	    	(59)
#define PARAM_OFFSET_6   		(PARAM_OFFSET_5 + PARAM_LENGTH_5)
#define PARAM_LENGTH_6			(2)
#define PARAM_MIN_6			    (5)
#define PARAM_MAX_6			    (200)

#define PARAM_NAME_7			Reserved//engine.OverloadDelayTime
#define PARAM_DAFAULT_7	    	(15)
#define PARAM_OFFSET_7   		(PARAM_OFFSET_6 + PARAM_LENGTH_6)
#define PARAM_LENGTH_7			(2)
#define PARAM_MIN_7			    (1)
#define PARAM_MAX_7			    (200)

#define PARAM_NAME_8			Reserved//engine.OverloadDelayTime
#define PARAM_DAFAULT_8	    	(100)
#define PARAM_OFFSET_8   		(PARAM_OFFSET_7 + PARAM_LENGTH_7)
#define PARAM_LENGTH_8			(4)
#define PARAM_MIN_8			    (1)
#define PARAM_MAX_8			    (60000)

#define PARAM_NAME_9			mixer.VolumeSlope//engine.OverloadDelayTime
#define PARAM_DAFAULT_9	    	(5)
#define PARAM_OFFSET_9   		(PARAM_OFFSET_8 + PARAM_LENGTH_8)
#define PARAM_LENGTH_9			(1)
#define PARAM_MIN_9			    (0)
#define PARAM_MAX_9			    (20)




#define PARAM_NAME_100			Password//engine.OverloadDelayTime
#define PARAM_DAFAULT_100	    (0x30)
#define PARAM_OFFSET_100   		(OFFSET_DEVICE_PARAM)
#define PARAM_LENGTH_100	    (LENGTH_PASSWORD * sizeof(uint8_t))
#define PARAM_MIN_100			(PASSWORD_CHAR_MIN)
#define PARAM_MAX_100			(PASSWORD_CHAR_MAX)//支持ASCII








#define PARAM_NAME(x)			PARAM_NAME_##x
#define PARAM_OFFSET(x)			PARAM_OFFSET_##x
#define PARAM_LENGTH(x)			PARAM_LENGTH_##x
#define PARAM_DEFAULT(x)			PARAM_DAFAULT_##x
#define PARAM_MIN(x)				PARAM_MIN_##x
#define PARAM_MAX(x)				PARAM_MAX_##x


int32_t ActiveSystemParam(uint8_t index)
{
    switch(index)
    {
        case 2:
            Tas5719SetVolume(mixer.VolumeLevel);
            break;
        case 4:
        case 5:
            //InitThrottleCurve();
            CalHalMaxSpeedCalibrate();
            break;
        default:
            break;
    }
    
    return 0;
}


//获取一个系统信息
//返回值为系统信息的内容长度
//如果Index错误，返回-1	
int32_t GetOneSystemParam(uint8_t *buff, uint8_t Index)
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
        default:
            return -1;
    }
    memcpy(buff, SrcPtr, length);
    return length;
}





/*****************************参数设定，直接修改内存中的参数变量*****************************************/
//先进行参数的检查，如果长度或内容不正常，则返回参数实际值
//如果设定参数正常，则改写内存中变量的参数值
//SetValuePtr:设定值的内存指针
//ParamValuePtr:设定后参数值的指针
//length:设定参数的长度
//Index:参数的索引
int32_t SetOneSystemParam(uint8_t *SetValuePtr, uint8_t *ParamValuePtr, uint8_t length, uint8_t Index)
{
    uint8_t *ParamAddr;
    int32_t ParamLength;
    switch(Index)
    {
//        case 1:
//            ParamAddr = (uint8_t *)&PARAM_NAME(1);
//            ParamLength = CheckParamIndex(SetValuePtr, PARAM_LENGTH(1), length, PARAM_NAME(1), PARAM_MIN(1), PARAM_MAX(1));
//            break;
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
        default:
            return -1;
    }

    memcpy(ParamAddr, SetValuePtr, ParamLength);
    memcpy(ParamValuePtr, SetValuePtr, ParamLength);

    ActiveSystemParam(Index);
    return ParamLength;
}


/***************************写入配置表，将内存中的各个变量重组成整张配置表，写入FLASH中********************************/


#define PARAM_TABLE_SIZE		WINBOND_FLASH_SECTOR_SIZE
int32_t WriteSystemParamTable()
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

    WriteOneParamToTable((unsigned char *)&PARAM_NAME(100), PARAM_OFFSET(100), PARAM_LENGTH(100), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(3), PARAM_OFFSET(3), PARAM_LENGTH(3), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(4), PARAM_OFFSET(4), PARAM_LENGTH(4), (unsigned char *)ParamTable);
        
//        WriteOneParam((unsigned char *)&PARAM_NAME(8), PARAM_OFFSET(8), PARAM_LENGTH(8), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(9), PARAM_OFFSET(9), PARAM_LENGTH(9), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(10), PARAM_OFFSET(10), PARAM_LENGTH(10), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(11), PARAM_OFFSET(11), PARAM_LENGTH(11), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(12), PARAM_OFFSET(12), PARAM_LENGTH(12), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(13), PARAM_OFFSET(13), PARAM_LENGTH(13), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(14), PARAM_OFFSET(14), PARAM_LENGTH(14), (unsigned char *)ParamTable);
//        
//        WriteOneParam((unsigned char *)&PARAM_NAME(51), PARAM_OFFSET(51), PARAM_LENGTH(51), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(52), PARAM_OFFSET(52), PARAM_LENGTH(52), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(53), PARAM_OFFSET(53), PARAM_LENGTH(53), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(54), PARAM_OFFSET(54), PARAM_LENGTH(54), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(55), PARAM_OFFSET(55), PARAM_LENGTH(55), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(56), PARAM_OFFSET(56), PARAM_LENGTH(56), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(57), PARAM_OFFSET(57), PARAM_LENGTH(57), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(58), PARAM_OFFSET(58), PARAM_LENGTH(58), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(59), PARAM_OFFSET(59), PARAM_LENGTH(59), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(60), PARAM_OFFSET(60), PARAM_LENGTH(60), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(61), PARAM_OFFSET(61), PARAM_LENGTH(61), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(62), PARAM_OFFSET(62), PARAM_LENGTH(62), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(63), PARAM_OFFSET(63), PARAM_LENGTH(63), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(64), PARAM_OFFSET(64), PARAM_LENGTH(64), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(65), PARAM_OFFSET(65), PARAM_LENGTH(65), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(66), PARAM_OFFSET(66), PARAM_LENGTH(66), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(67), PARAM_OFFSET(67), PARAM_LENGTH(67), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(68), PARAM_OFFSET(68), PARAM_LENGTH(68), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(69), PARAM_OFFSET(69), PARAM_LENGTH(69), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(70), PARAM_OFFSET(70), PARAM_LENGTH(70), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(71), PARAM_OFFSET(71), PARAM_LENGTH(71), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(72), PARAM_OFFSET(72), PARAM_LENGTH(72), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(73), PARAM_OFFSET(73), PARAM_LENGTH(73), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(74), PARAM_OFFSET(74), PARAM_LENGTH(74), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(75), PARAM_OFFSET(75), PARAM_LENGTH(75), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(76), PARAM_OFFSET(76), PARAM_LENGTH(76), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(77), PARAM_OFFSET(77), PARAM_LENGTH(77), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(78), PARAM_OFFSET(78), PARAM_LENGTH(78), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(79), PARAM_OFFSET(79), PARAM_LENGTH(79), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(80), PARAM_OFFSET(80), PARAM_LENGTH(80), (unsigned char *)ParamTable);
//        
//        WriteOneParam((unsigned char *)&PARAM_NAME(151), PARAM_OFFSET(151), PARAM_LENGTH(151), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(152), PARAM_OFFSET(152), PARAM_LENGTH(152), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(153), PARAM_OFFSET(153), PARAM_LENGTH(153), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(154), PARAM_OFFSET(154), PARAM_LENGTH(154), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(155), PARAM_OFFSET(155), PARAM_LENGTH(155), (unsigned char *)ParamTable);
//        WriteOneParam((unsigned char *)&PARAM_NAME(156), PARAM_OFFSET(156), PARAM_LENGTH(156), (unsigned char *)ParamTable);

    CheckSum = CalSum((uint32_t *)ParamTable, (PARAM_TABLE_SIZE >> 2) - 1);
    memcpy(ParamTable + PARAM_TABLE_SIZE - 4, &CheckSum, 4);

    WriteParamSector(ParamTable, SectorAddr(SECTOR_INDEX_SYSTEM_CONFIG));
    return 0;
}





void AnalyzeSystemConfig()
{
    uint8_t SystemConfigBuff[512];
    
    
    DataFlashReadData(SectorAddr(SECTOR_SYSTEM_CONFIG), SystemConfigBuff, 512);
   
    InitOneParam(SystemConfigBuff, (unsigned char *)&PARAM_NAME(1), PARAM_OFFSET(1), PARAM_LENGTH(1), PARAM_DEFAULT(1), PARAM_MIN(1), PARAM_MAX(1));
    InitOneParam(SystemConfigBuff, (unsigned char *)&PARAM_NAME(2), PARAM_OFFSET(2), PARAM_LENGTH(2), PARAM_DEFAULT(2), PARAM_MIN(2), PARAM_MAX(2));
    InitOneParam(SystemConfigBuff, (unsigned char *)&PARAM_NAME(3), PARAM_OFFSET(3), PARAM_LENGTH(3), PARAM_DEFAULT(3), PARAM_MIN(3), PARAM_MAX(3));
    InitOneParam(SystemConfigBuff, (unsigned char *)&PARAM_NAME(4), PARAM_OFFSET(4), PARAM_LENGTH(4), PARAM_DEFAULT(4), PARAM_MIN(4), PARAM_MAX(4));
    InitOneParam(SystemConfigBuff, (unsigned char *)&PARAM_NAME(5), PARAM_OFFSET(5), PARAM_LENGTH(5), PARAM_DEFAULT(5), PARAM_MIN(5), PARAM_MAX(5));
    InitOneParam(SystemConfigBuff, (unsigned char *)&PARAM_NAME(6), PARAM_OFFSET(6), PARAM_LENGTH(6), PARAM_DEFAULT(6), PARAM_MIN(6), PARAM_MAX(6));
    InitOneParam(SystemConfigBuff, (unsigned char *)&PARAM_NAME(7), PARAM_OFFSET(7), PARAM_LENGTH(7), PARAM_DEFAULT(7), PARAM_MIN(7), PARAM_MAX(7));
    InitOneParam(SystemConfigBuff, (unsigned char *)&PARAM_NAME(8), PARAM_OFFSET(8), PARAM_LENGTH(8), PARAM_DEFAULT(8), PARAM_MIN(8), PARAM_MAX(8));
    InitOneParam(SystemConfigBuff, (unsigned char *)&PARAM_NAME(9), PARAM_OFFSET(9), PARAM_LENGTH(9), PARAM_DEFAULT(9), PARAM_MIN(9), PARAM_MAX(9));

    //InitThrottleCurve();
    CalHalMaxSpeedCalibrate();
}

/** 
 * [AnalyzeDeviceConfig description]进行设配配置的初始化，包括设备密码
 * @Author   tin39
 * @DateTime 2019年3月12日T15:58:27+0800
 * @param                             [description]
 */
void AnalyzeDeviceConfig()
{
    uint8_t DeviceConfigBuff[512];
    DataFlashReadData(SectorAddr(SECTOR_SYSTEM_CONFIG) + OFFSET_DEVICE_PARAM, DeviceConfigBuff, 512);
    InitBuffParam(DeviceConfigBuff, (unsigned char *)&PARAM_NAME(100), PARAM_OFFSET(100) - OFFSET_DEVICE_PARAM, PARAM_LENGTH(100), PARAM_DEFAULT(100), PARAM_MIN(100), PARAM_MAX(100));
}



