#include "SystemHw.h"
#include "SystemInfo.h"
#include "Fs.h"
#include "engine.h"
#include "SystemConfig.h"
#include "MixerConfig.h"
#include "OnChipAudio.h"
#include "mixer.h"
#include "MotorSpeed.h"
#include "OnChipLed.h"
#include "key.h"
#include "ActionTick.h"
#include "BleTransportLayer.h"

#include "DownLoader.h"

#include "DecryDesKey.h"

#include "Authorize.h" 
#include "engine.h"
#include "CalibrationMode.h"


int main(void)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                       
{  
	IWDG_ReloadCounter();
    InitAllPeriph();
    GetDeviceId();

    //ChipErase();
    InitFs();

    DownloaderInit();
    GetRsaPrivateKey();
    GetAuthorizeRSAPulbicKey();

    //启动延时500ms，防止上电的同时，功放工作，造成浪涌烧坏功放
    mDelay(51);

    
    AnalyzeSystemConfig();
    AnalyzeDeviceConfig();

    bool IsCalibrationMode;
    IsCalibrationMode =  CalibrationCheck();
    if(IsCalibrationMode == true)
    {
        CalibrationMode();
    }
    
	
    //配置的解析
    
    //TryChangeSoundIndex(mixer.SoundIndex);
    TryChangeVehileIndexLoop(mixer.SoundIndex, (bool)TRUE);
    

    IWDG_ReloadCounter();


    //For tests
    //PlaySystemStartBeep(0);
    //mDelay(1000);

    /*if(IsDeviceProduced() != 0)
    {
        AmplifierEnable();
        //设备已被生产工具初始化，进行滴声的播放
        PlaySystemStartBeep(mixer.SoundIndex);
        //延时1秒，防止滴滴声和引擎启东声混在一起
        mDelay(1000);
    }*/



    //ConvertTest();

    
    //StartCheckAmplifierVoltage();
    //上电就启动
    EngineStartHandler();

    //Md5Test();
    

    while(1)
    {
        EngineHandler();

        KeyTopLevelHandler();
        //SlientKeyHandler();
        

        IWDG_ReloadCounter();
        AmplifierControlHandler();



        BleAnalyzeTransportLayer();

        MixerBleTopLevelHandler();

        EvMotorSpeedHandler();



        VolumeHandler();

        OnChipLedHandler();

    }

}




