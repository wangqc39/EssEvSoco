#include "main.h"
#include "BleTransportLayer.h"


int main(void)
{  
    volatile int ret;
    

    Init_All_Periph();

     IWDG_ReloadCounter();
	

    
																		       
        if(BootCheck() == BOOT_BOOTLOADER)
        { 
                ActionTickHwInit();
                SetBootData(BOOT_APP);
                GetDeviceId();

                BleComHwInit();
                SpiFlashHwInit();
                DownloaderInit();

                while(1)
                {
                    BleAnalyzeTransportLayer();
                    IWDG_ReloadCounter();
                }
        }
        else
        {
            //while(1);
            //BKP中值为0直接进入APP
            RunToApplication(APPLICATION_ADDRESS);
        }
        

    


    
}




