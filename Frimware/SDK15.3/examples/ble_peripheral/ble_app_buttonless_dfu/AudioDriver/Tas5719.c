#include "nrf_twi_mngr.h"
#include "common.h"
#include "i2c.h"
#include "Tas5719.h"
#include "ActionTick.h"
#include "Mixer.h"

#define I2C_ADDR_TAS5719        (0x54 >> 1)




/** 
 * [TAS5719_ReadReg description]
 * @Author   tin39
 * @DateTime 2019年7月3日T11:12:57+0800
 * @param    RegAddr                  [description]TAS5719寄存器地址
 * @param    ReadCnt                  [description]读取数据的字节数
 * @return                            [description]返回读取内容，小端返回
 */
uint32_t TAS5719_ReadReg(uint8_t RegAddr, uint8_t ReadCnt)
{
    uint8_t ReadBuff[4];
    uint32_t RetData = 0;
    nrf_twi_mngr_transfer_t transfers[2] = 
    {
        NRF_TWI_MNGR_WRITE(I2C_ADDR_TAS5719, &RegAddr, sizeof(RegAddr), 0),
        NRF_TWI_MNGR_READ(I2C_ADDR_TAS5719, ReadBuff, ReadCnt, 0)
    };
    
    I2cOpBlocking(transfers, 2);

    for(int i = 0; i < ReadCnt; i++)
    {
        RetData |= ((uint32_t)ReadBuff[ReadCnt - i - 1] << (i * 8));
    }

    return RetData;
}

/** 
 * [TAS5719_WriteReg description]
 * @Author   tin39
 * @DateTime 2019年7月3日T11:11:22+0800
 * @param    RegAddr                  [description]TAS5719寄存器地址
 * @param    data                     [description]写入的数据，数据支持最大32bit，数据内容为小端，发送时需要转换成大端
 * @param    DataCnt                  [description]写入数据的字节数
 */
void TAS5719_WriteReg(uint8_t RegAddr, uint32_t data, uint8_t DataCnt)
{
    uint8_t WriteBuff[5];
    WriteBuff[0] = RegAddr;
    for(int i = 0; i < DataCnt; i++)
    {
        WriteBuff[1 + i] = (uint8_t)(data >> ((DataCnt - 1) * 8 - i * 8));
    }
    nrf_twi_mngr_transfer_t transfers[1] = 
    {
        NRF_TWI_MNGR_WRITE(I2C_ADDR_TAS5719, WriteBuff, DataCnt + 1, 0),
        //NRF_TWI_MNGR_WRITE(I2C_ADDR_TAS5719, DataBuff, DataCnt, 0)
    };
    I2cOpBlocking(transfers, 1);
}

void TAS5719_Init()
{
    I2cInit();
    
    //具体参见SENSE-ESS2-TAS5719驱动
    //44.1sample rate;MCLK frequency = 64 × fS
    TAS5719_WriteReg(0, (3 << 5) | (0 << 2), 1);
    //TAS5719_WriteReg(0, (3 << 5) | (1 << 2), 1);
    //PWM high-pass (dc blocking) enabled;Hard unmute on recovery from clock error;No de-emphasis
    TAS5719_WriteReg(3, (1 << 7) | (1 << 5) | (0 << 0), 1);
    //I2S;16bit word length
    //TAS5719_WriteReg(4, 4, 1);
    TAS5719_WriteReg(4, 3, 1);
    //Enter all-channel shutdown (hard mute);Speaker Mode;not in ternary modulation;Internal power stage FAULT signal is the source of A_SEL/HP_SD pi
    TAS5719_WriteReg(5, (1 << 6) | (0 << 4) | (0 << 3) | (0 << 1) | (0 << 0), 1);
    //由于设置了Hard unmute，所以这里全部为0
    TAS5719_WriteReg(6, 0, 1);
    //音量设置
    TAS5719_WriteReg(7, 0xC0, 2);
    TAS5719_WriteReg(8, 0xC0, 2);
    TAS5719_WriteReg(9, 0xC0, 2);
    //Volume slew 2048 steps (171-ms volume ramp time at 48 kHz)
    TAS5719_WriteReg(0x0E, 0x92, 1);
    //Modulation Limit:97.7
    TAS5719_WriteReg(0x10, 0x02, 1);
    //ICD to default value of BD MODE
    TAS5719_WriteReg(0x11, 0xBC, 1);
    TAS5719_WriteReg(0x12, 0x60, 1);
    TAS5719_WriteReg(0x13, 0xA0, 1);
    TAS5719_WriteReg(0x14, 0x48, 1);
    //PBTL mode
    //TAS5719_WriteReg(0x19, 0x30, 1);
    TAS5719_WriteReg(0x19, 0x3A, 1);
    //125.7-ms 50% duty cycle start/stop period
    TAS5719_WriteReg(0x1A, 0x0F, 1);
    //Select factory trim
    TAS5719_WriteReg(0x1B, 0, 1);
    //Headphone enable time = 10 ms;Set back-end reset period to 299 ms
    TAS5719_WriteReg(0x1C, 0x52, 1);
    //Channel-1 BD mode;SDIN-L to channel 1;Channel 2 BD mode;SDIN-R to channel 2
    TAS5719_WriteReg(0x20, (1 << 23) | (0 << 20) | (1 << 19) | (1 << 16) | 0x7772, 4);
    //Channel 4 source Left-channel post-BQ
    TAS5719_WriteReg(0x21, 0x00004303, 4);
    //Multiplex channel 1 to OUT_A;Multiplex channel 1 to OUT_B;Multiplex channel 2 to OUT_C;Multiplex channel 2 to OUT_D
    //AS5719_WriteReg(0x25, 0x01021345, 4);
    TAS5719_WriteReg(0x25, 0x01002245, 4);
    //DRC turned OFF(Dynamic Range Compression 动态范围压缩)
    TAS5719_WriteReg(0X46, 0x00000020, 4);
    //PWM SWITCHING RATE CONTROL REGISTER；SRC = 6；文档没有明确定义，使用默认值
    TAS5719_WriteReg(0x4F, 0x00000006, 4);
    //EQ ON;L and R can be written independently
    TAS5719_WriteReg(0x50, 0x00000000, 4);



    //Exit all-channel shutdown;Speaker Mode;not in ternary modulation;Internal power stage FAULT signal is the source of A_SEL/HP_SD pi
    //TAS5719的实际开启由Tas5719Cmd方法进行
    //TAS5719_WriteReg(5, (0 << 6) | (0 << 4) | (0 << 3) | (0 << 1) | (0 << 0), 1);
    

    
   
}



void Tas5719Cmd(FunctionalState CmdState)
{
    if(CmdState == DISABLE)

    {
        TAS5719_WriteReg(5, (1 << 6) | (0 << 4) | (0 << 3) | (0 << 1) | (0 << 0), 1);
    }
    else
    {
        TAS5719_WriteReg(5, (0 << 6) | (0 << 4) | (0 << 3) | (0 << 1) | (0 << 0), 1);
    }
}

#define VOLUME_DB_MAX           24
#define VOLUME_DB_INTERVAL    2
void Tas5719SetVolume(uint8_t VolumeLevel)
{
    uint16_t VolumeSet;
    int32_t VolumeDb;
    if(VolumeLevel == 0)
    {
        TAS5719_WriteReg(7, 0x3FF, 2);
    }
    else
    {
        VolumeDb = VOLUME_DB_MAX - (int32_t)(MAX_VOLUME - VolumeLevel) * VOLUME_DB_INTERVAL;
        VolumeSet = (24 - VolumeDb) * 8;
        TAS5719_WriteReg(7, VolumeSet, 2);
    }
}



uint8_t TasError;
uint8_t Reg0;
uint8_t WorkStatus;
void TasDebug()
{
    static uint32_t LastHandlerTime = 0;
    if(GetSystemTime() < LastHandlerTime + 1000 * TIMER_MULTI)
        return;
    LastHandlerTime = GetSystemTime();
    
    TasError = TAS5719_ReadReg(2, 1);
    Reg0 = TAS5719_ReadReg(0, 1);
    WorkStatus = TAS5719_ReadReg(5, 1);
    TAS5719_WriteReg(2, 0, 1);
}

