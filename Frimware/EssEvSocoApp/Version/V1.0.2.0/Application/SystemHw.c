#include "common.h"
#include "SystemInfo.h"
#include "SpiFlash.h"
#include "ActionTick.h"
#include "key.h"
#include "mixer.h"
#include "engine.h"
#include "OnChipLed.h"

#include "BleComHw.h"
#include "MotorSpeed.h"

#include "MotorSpeedHal.h"





void RCC_Configuration_HSI()
{
        RCC_DeInit();

        RCC_HSEConfig(RCC_HSE_OFF);

        RCC_HSICmd(ENABLE);                        //打开内部时钟
        
        while(RCC_GetFlagStatus(RCC_FLAG_HSIRDY) == RESET)        
        {        
        }

        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

        FLASH_SetLatency(FLASH_Latency_2);
        /* HCLK = SYSCLK */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
        //APB2
        RCC_PCLK2Config(RCC_HCLK_Div1);
        //APB1
        RCC_PCLK1Config(RCC_HCLK_Div2);
        //PLL 倍频
        RCC_PLLConfig(RCC_PLLSource_HSI_Div2, RCC_PLLMul_16);        //64M
        RCC_PLLCmd(ENABLE);                        //使能倍频
                                                                                                         
   //等待指定的 RCC 标志位设置成功 等待PLL初始化成功
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }

        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);        
        while(RCC_GetSYSCLKSource() != 0x08){}
}


void RccConfiguration(void)
{
    
    RCC_ClocksTypeDef RCC_Clocks;
    
    //将外设 RCC寄存器重设为缺省值
    
    ErrorStatus HSEStartUpStatus;
    RCC_DeInit();
    //设置外部高速晶振（HSE）
    RCC_HSEConfig(RCC_HSE_ON);
  
    //等待 HSE 起振 
    HSEStartUpStatus = RCC_WaitForHSEStartUp();
  
    if(HSEStartUpStatus == SUCCESS)
    {
        //预取指缓存使能
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    
         //设置代码延时值
        //FLASH_Latency_2  2 延时周期
        FLASH_SetLatency(FLASH_Latency_2);
    
        //设置 AHB 时钟（HCLK）
        //RCC_SYSCLK_Div1  AHB 时钟 =  系统时钟 
        RCC_HCLKConfig(RCC_SYSCLK_Div1);
    
         //设置高速 AHB 时钟（PCLK2）
        //RCC_HCLK_Div2  APB1 时钟  = HCLK / 2 
        RCC_PCLK2Config(RCC_HCLK_Div1);
    
        //设置低速 AHB 时钟（PCLK1）
        //RCC_HCLK_Div2  APB1 时钟  = HCLK / 2 
        RCC_PCLK1Config(RCC_HCLK_Div2);
    
        // PLLCLK = 8MHz * 9 = 72 MHz 
        //设置 PLL 时钟源及倍频系数
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
    
        //使能或者失能 PLL
        RCC_PLLCmd(ENABLE);
    
        //等待指定的 RCC 标志位设置成功 等待PLL初始化成功
        while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
        {
        }
    
    
        //设置系统时钟（SYSCLK） 设置PLL为系统时钟源
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
    
        //等待PLL成功用作于系统时钟的时钟源
        //  0x00：HSI 作为系统时钟 
        //  0x04：HSE作为系统时钟 
        //  0x08：PLL作为系统时钟  
        while(RCC_GetSYSCLKSource() != 0x08)
        {
        }
    }
    else
    {
        RCC_Configuration_HSI();
    }
    
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA 
                             |RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC
                             |RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOE
  						   |RCC_APB2Periph_ADC1  | RCC_APB2Periph_AFIO 
                             , ENABLE );
  
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    RCC_GetClocksFreq(&RCC_Clocks);
}




void NvicConfiguration(void)
{ 
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, MAIN_APP_START_ADDR - 0x08000000);   
    //NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);   
    
    /* Configure the NVIC Preemption Priority Bits */  
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}



void SetBootData(BootFromTypeInfo BootData)
{
    BKP_WriteBackupRegister(BKP_DR1, (unsigned int)BootData);
    //RTC_WriteBackupRegister(RTC_BKP_DR0, (unsigned int)BootData);
}

void CodeFlashProtect()
{
    
    if(FLASH_GetReadOutProtectionStatus() == RESET)
    {
        FLASH_Unlock();
        FLASH_ReadOutProtection(ENABLE);
        FLASH_Lock();
    }
    
}

void BkpHwInit()
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_BKP, ENABLE);
    PWR_BackupAccessCmd(ENABLE);
}

void IwdgInit(void)  
{
         
	  /* Enable write access to IWDG_PR and IWDG_RLR registers */
	  IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
	  /* IWDG counter clock: 40KHz(LSI) / 256 = 156.25Hz */
	  IWDG_SetPrescaler(IWDG_Prescaler_256);//8ms
	  /* Set counter reload value to 4095 */
	  IWDG_SetReload(1);//超时时间= (256/(40*1000))*313=2.0032s
	  /* Reload IWDG counter */
	  IWDG_ReloadCounter();
	  /* Enable IWDG (the LSI oscillator will be enabled by hardware) */
	  IWDG_Enable();
}





void InitAllPeriph(void)
{

    //CodeFlashProtect();
    //系统时钟设定
    RccConfiguration();
    NvicConfiguration();
    BkpHwInit();
    ItIsASecret();
    //备份寄存器设定，用于启动时指定驻留在BOOTLOADER还是APP
    SetBootData(BOOT_APP);
    

    SystemTickConfiguration();
    OnChipLedHwInit();
    
    //FLASH初始化
    SpiFlashHwInit();


    

    KeyHwInit();
    //SlientKeyHwInit();

    
    EngineMixerChannelInit();
    AudioOutHwConfig();


    BleComHwInit();

    EvMotorSpeedHwInit();


    MotorSpeedHalHwInit();
    
}

/****************************RC PLUS数据监控*****************************/
/*void RunToRcPlusConnector()
{
    typedef  void (*pFunction)(void);
    unsigned int JumpAddress;
    pFunction Jump_To_Application;
    unsigned Addr;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 | RCC_APB1Periph_TIM14 |RCC_APB1Periph_USART2 | RCC_APB1Periph_DAC | RCC_APB1Periph_SPI2, DISABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_TIM1 | RCC_APB2Periph_USART1 | RCC_APB2Periph_TIM15 | RCC_APB2Periph_TIM16 | RCC_APB2Periph_TIM17, DISABLE);


    Addr = CONNECTOR_START_ADDR;

    JumpAddress = *(volatile unsigned int*) (Addr + 4);

      
    Jump_To_Application = (pFunction) JumpAddress;//Jump to user application 

    IWDG_ReloadCounter();
      
    __set_MSP(*(volatile unsigned int*) Addr);//Initialize user application's Stack Pointer 
    Jump_To_Application();   
}



void RcPlusConnectMonitor()
{
    if(RcCapture.ContinueRcPlusData >= 3)
    {
        RunToRcPlusConnector();
    }
}*/

