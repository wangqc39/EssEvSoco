# BIM OAD On-Chip

## Table of Contents

* [Introduction](#Introduction)
* [Hardware Prerequisites](#Hardware Prerequisites)
* [Software Prerequisites](#Software Prerequisites)
* [Usage](#Usage)

## <a name="Introduction"></a>Introduction

The bim\_oad\_onchip is a small project used with the oad\_target project; it is
not used as a standalone project. It is a lightweight Boot Image Manager (BIM)
that resides in the last page of internal flash and executes after the boot ROM.
BIM for on-chip OAD is used to switch between image A and B when performing an
on-chip OAD. See the [**OAD section of the Bluetooth low
energy Software Developer's Guide**](../../../../../docs/blestack/ble_sw_dev_guide/html/oad/oad.html)
for more information.

## <a name="Hardware Prerequisites"></a>Hardware Prerequisites

The default BIM OAD On-chip configuration uses the
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

The usage of this project is explained in the
[**OAD section of the Bluetooth low energy Software Developer's
Guide**](../../../../../docs/blestack/ble_sw_dev_guide/html/oad/oad.html).
