#
#  Do not edit this file.  This file is generated from 
#  package.bld.  Any modifications to this file will be 
#  overwritten whenever makefiles are re-generated.
#
#  target compatibility key = iar.targets.arm.M3{1,0,7.80,4
#
ifeq (,$(MK_NOGENDEPS))
-include package/cfg/app_ble_prm3.orm3.dep
package/cfg/app_ble_prm3.orm3.dep: ;
endif

package/cfg/app_ble_prm3.orm3: | .interfaces
package/cfg/app_ble_prm3.orm3: package/cfg/app_ble_prm3.c package/cfg/app_ble_prm3.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm3 $< ...
	LC_ALL=C $(iar.targets.arm.M3.rootDir)/bin/iccarm    -D CC2640R2_LAUNCHXL   -D CC26XX   -D CC26XX_R2   -D DEVICE_FAMILY=cc26x0r2   -D Display_DISABLE_ALL   -D FEATURE_OAD   -D FEATURE_OAD_ONCHIP   -D HAL_IMAGE_A   -D HEAPMGR_SIZE=0   -D ICALL_EVENTS   -D ICALL_JT   -D ICALL_LITE   -D ICALL_MAX_NUM_ENTITIES=6   -D ICALL_MAX_NUM_TASKS=3   -D MAX_NUM_BLE_CONNS=1   -D xPOWER_SAVING   -D USE_CORE_SDK   -D USE_ICALL   -D xdc_runtime_Assert_DISABLE_ALL   -D xdc_runtime_Log_DISABLE_ALL   -D USE_RCOSC   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/controller/cc26xx_r2/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/common/cc26xx/   -I T:/OneDrive/SoundSimulator2/EssEv/EssEvMicorMax/Firmware/BLE-MOD/oad_target/tirtos/iar/app/../../../src/app/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/icall/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/profiles/oad/cc26xx/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/profiles/roles/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/profiles/roles/cc26xx/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/target/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/hal/src/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/hal/src/target/_common/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/hal/src/target/_common/cc26xx/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/hal/src/target/cc2650/rom/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/heapmgr/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/icall/src/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/osal/src/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/services/src/saddr/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/services/src/sdata/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/devices/cc26x0r2/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/common/cc26xx/rcosc/   --silent --aeabi --cpu=Cortex-M3 --diag_suppress=Pa050,Go005 --endian=little -e --thumb   -Dxdc_cfg__xheader__='"configPkg/package/cfg/app_ble_prm3.h"'  -Dxdc_target_name__=M3 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_release -Dxdc_bld__vers_1_0_7_80_4 -Ohs --dlib_config $(iar.targets.arm.M3.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

package/cfg/app_ble_prm3.srm3: | .interfaces
package/cfg/app_ble_prm3.srm3: package/cfg/app_ble_prm3.c package/cfg/app_ble_prm3.mak
	@$(RM) $@.dep
	$(RM) $@
	@$(MSG) clrm3 $< ...
	LC_ALL=C $(iar.targets.arm.M3.rootDir)/bin/iccarm    -D CC2640R2_LAUNCHXL   -D CC26XX   -D CC26XX_R2   -D DEVICE_FAMILY=cc26x0r2   -D Display_DISABLE_ALL   -D FEATURE_OAD   -D FEATURE_OAD_ONCHIP   -D HAL_IMAGE_A   -D HEAPMGR_SIZE=0   -D ICALL_EVENTS   -D ICALL_JT   -D ICALL_LITE   -D ICALL_MAX_NUM_ENTITIES=6   -D ICALL_MAX_NUM_TASKS=3   -D MAX_NUM_BLE_CONNS=1   -D xPOWER_SAVING   -D USE_CORE_SDK   -D USE_ICALL   -D xdc_runtime_Assert_DISABLE_ALL   -D xdc_runtime_Log_DISABLE_ALL   -D USE_RCOSC   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/controller/cc26xx_r2/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/common/cc26xx/   -I T:/OneDrive/SoundSimulator2/EssEv/EssEvMicorMax/Firmware/BLE-MOD/oad_target/tirtos/iar/app/../../../src/app/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/icall/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/profiles/oad/cc26xx/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/profiles/roles/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/profiles/roles/cc26xx/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/target/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/hal/src/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/hal/src/target/_common/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/hal/src/target/_common/cc26xx/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/hal/src/target/cc2650/rom/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/heapmgr/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/icall/src/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/osal/src/inc/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/services/src/saddr/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/services/src/sdata/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/devices/cc26x0r2/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/   -I C:/ti/simplelink_cc2640r2_sdk_1_30_00_25/source/ti/blestack/common/cc26xx/rcosc/   --silent --aeabi --cpu=Cortex-M3 --diag_suppress=Pa050,Go005 --endian=little -e --thumb   -Dxdc_cfg__xheader__='"configPkg/package/cfg/app_ble_prm3.h"'  -Dxdc_target_name__=M3 -Dxdc_target_types__=iar/targets/arm/std.h -Dxdc_bld__profile_release -Dxdc_bld__vers_1_0_7_80_4 -Ohs --dlib_config $(iar.targets.arm.M3.rootDir)/inc/c/DLib_Config_Normal.h  $(XDCINCS)  -o $@  $<
	
	-@$(FIXDEP) $@.dep $@.dep
	

clean,rm3 ::
	-$(RM) package/cfg/app_ble_prm3.orm3
	-$(RM) package/cfg/app_ble_prm3.srm3

app_ble.prm3: package/cfg/app_ble_prm3.orm3 package/cfg/app_ble_prm3.mak

clean::
	-$(RM) package/cfg/app_ble_prm3.mak
