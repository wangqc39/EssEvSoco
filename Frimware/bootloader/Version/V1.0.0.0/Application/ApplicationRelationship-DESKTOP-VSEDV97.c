#include "main.h"

typedef  void (*pFunction)(void);
unsigned int JumpAddress;
pFunction Jump_To_Application;


__asm void __set_MSP(unsigned int topOfMainStack)
{
  //__asm("msr msp, r0");
  //__asm("bx lr");

  msr msp, r0
  bx lr
}




void RunToApplication(unsigned int Addr)
{
     //如果是跑到APP里面的，则把BKP的寄存器置为1，APP里面会将其清零
    if(Addr == APPLICATION_ADDRESS)
    {
        SetBootData(BOOT_BOOTLOADER);
        //IwdgInit();
    }
    
    
    //RCC_APB1PeriphClockCmd(RCC_APB1Periph_WWDG, DISABLE);         
    JumpAddress = *(volatile unsigned int*) (Addr + 4);
    
    
    Jump_To_Application = (pFunction) JumpAddress;//Jump to user application 
    
    IWDG_ReloadCounter();
    
    __set_MSP(*(volatile unsigned int*) Addr);//Initialize user application's Stack Pointer 
    Jump_To_Application();   
}


unsigned int OnChipFlashPagesMask(volatile unsigned int Size)
{
  unsigned int pagenumber = 0x0;
  unsigned int size = Size;

  if ((size % STM32_PAGE_SIZE) != 0)
  {
    pagenumber = (size / STM32_PAGE_SIZE) + 1;
  }
  else
  {
    pagenumber = size / STM32_PAGE_SIZE;
  }
  return pagenumber;

}


