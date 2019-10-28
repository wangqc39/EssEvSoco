# invoke SourceDir generated makefile for app_ble.prm3
app_ble.prm3: .libraries,app_ble.prm3
.libraries,app_ble.prm3: package/cfg/app_ble_prm3.xdl
	$(MAKE) -f T:\OneDrive\SoundSimulator2\EssEv\EssEvMicorMax\Firmware\BLE-MOD\oad_target\tirtos\iar\config/src/makefile.libs

clean::
	$(MAKE) -f T:\OneDrive\SoundSimulator2\EssEv\EssEvMicorMax\Firmware\BLE-MOD\oad_target\tirtos\iar\config/src/makefile.libs clean

