#ifndef __SYSTEM_ERROR__
#define __SYSTEM_ERROR__

//每8位表示一盏灯
//其中0表示常亮，1表示闪烁1下,0xFF表示长灭
typedef enum {NO_ERROR = 0, CONFIG_VERSION_ERROR = 0x01FF03FF, 
                        BATTERY_NONE_ERROR = 0x01FF01FF, AUDIO_FILE_ERROR = 0x02FF01FF, BATTERY_LOW_ERROR = 0x02FF02FF,
                        BATTERY_HIGH_ERROR = 0x02FF03FF, START_BATTERY_HIGH_ERROR = 0x04FF04FF} SystemErrorFlag;

//变量按照优先级的顺序进行排列
struct ErrorInfo
{
    SystemErrorFlag MostSeriousError;
    SystemErrorFlag ConfigVersionrror;//配置信息版本错误
    SystemErrorFlag AudioFileError;//Flash中的语音数据错误
    SystemErrorFlag BatteryNoneError;//无功放电池错误
    SystemErrorFlag BatteryLowError;
    SystemErrorFlag BatteryHighError;
    SystemErrorFlag StartBatteryHighError;
};


extern struct ErrorInfo SystemError;

void SetSystmError(SystemErrorFlag ErrorType);
void ClearSystemError(SystemErrorFlag ErrorType);


#endif

