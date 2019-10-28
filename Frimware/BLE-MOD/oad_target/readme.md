# OAD Target

## Table of Contents

* [Introduction](#Introduction)
* [Hardware Prerequisites](#Hardware Prerequisites)
* [Software Prerequisites](#Software Prerequisites)
* [Usage](#Usage)

## <a name="Introduction"></a>Introduction

The oad\_target is used for on-chip Over the Air Download (OAD) firmware
update, and is to be used with the Boot Image Manager (BIM) project. See the
[**OAD section of the Bluetooth low energy Software Developer's
Guide**](../../../../../docs/blestack/ble_sw_dev_guide/html/oad/oad.html#on-chip-oad)
for more information.

## <a name="Hardware Prerequisites"></a>Hardware Prerequisites

The default OAD Target configuration uses the
[LAUNCHXL-CC2640R2](http://www.ti.com/tool/launchxl-cc2640r2). This hardware
configuration is shown in the below image:

<img src="resource/hardware_setup.jpg" width="300" height="400" />

For custom hardware, see the [**Running the SDK on Custom Boards section of the
TI Bluetooth low energy Software Developer's Guide**](../../../../../docs/blestack/ble_sw_dev_guide/html/ble-stack/index.html#running-the-sdk-on-custom-boards).

## <a name="Software Prerequisites"></a>Software Prerequisites

For information on what versions of Code Composer Studio and IAR Embedded
Workbench to use, see the Release Notes located in the
docs/blestack folder. For
information on how to import this project into your IDE workspace and
build/run, please refer to [**The CC2640R2F Platform section in the Bluetooth
low energy Software Developer's Guide**](../../../../../docs/blestack/ble_sw_dev_guide/html/cc2640/platform.html).

## <a name="Usage"></a>Usage

This application acts as a peripheral device that can be used as the target
device for OAD. It advertises upon startup, and a central device can connect
with it to initiate an OAD. It contains the OAD profile, and using it with the
bim_oad_onchip project allows for an image sent over the air to the device to be
downloaded without using any external memory. By default, it has no peripherals
(buttons, display). For details on using this project for on-chip OAD, see the
[**OAD section of the Bluetooth low energy Software Developer's
Guide**](../../../../../docs/blestack/ble_sw_dev_guide/html/oad/oad.html#on-chip-oad).
