/******************** (C) COPYRIGHT 2007 STMicroelectronics ********************
* File Name          : readme.txt
* Author             : MCD Application Team
* Version            : V1.0
* Date               : 10/08/2007
* Description        : Description of the I2C Example2.
********************************************************************************
* THE PRESENT SOFTWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH SOFTWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/
本例展示了如何为I2C2设置2个不同地址的，并通过I2C1把2块不同数据发向这2个地址，显然这些数据都为I2C2所接收。

程序分为2步：
1.  I2C1向第一个从I2C地址I2C2_SLAVE1_ADDRESS7发送数据，把I2C1 Tx_Buffer1中的数据发向I2C2，I2C2接收到的数据存放在I2C2 Rx_Buffer1中。I2C2的第一个从地址I2C2_SLAVE1_ADDRESS7存放在I2C2 OAR1寄存器（I2C2 OAR1 register）中。传送完成后，检查传送是否正确。

2. I2C1向第二个从I2C地址I2C2_SLAVE2_ADDRESS7发送数据，把I2C1 Tx_Buffer2中的数据发向I2C2，I2C2接收到的数据存放在I2C2 Rx_Buffer3中。I2C2的第二个从地址I2C2_SLAVE2_ADDRESS7存放在I2C2 OAR2寄存器（I2C2 OAR2 register）中。传送完成后，检查传送是否正确。

Example description
===================
This example provides a description of how to transfer two data buffer from I2C1 to
I2C2 through its two address in the same application.

Dual addressing implies two steps:
1. First, the I2C1 master transmitter sends the I2C1 Tx_Buffer1 data buffer to
   the slave (I2C2) that saves the received data in I2C2 Rx_Buffer1. I2C2 is
   addressed by its first slave address I2C2_SLAVE1_ADDRESS7 programmed in the
   I2C2 OAR1 register.
   These transmitted and received buffers are compared to check that all data
   have been correctly transferred.
2. Second, the I2C2 is now addressed by its second slave address I2C2_SLAVE2_ADDRESS7
   programmed in the I2C2 OAR2 register. The I2C1 Tx_Buffer2 contents are transmitted
   by the master (I2C1) to the slave (I2C2) that stores them into I2C2 Rx_Buffer2.
   A second comparison takes place between the transmitted

The communication clock speed is set to 200KHz.


Directory contents
==================
stm32f10x_conf.h  Library Configuration file
stm32f10x_it.c    Interrupt handlers
stm32f10x_it.h    Interrupt handlers header file
main.c            Main program


Hardware environment
====================
 - Connect I2C1 SCL pin (PB.06) to I2C2 SCL pin (PB.10)
 - Connect I2C1 SDA pin (PB.07) to I2C2 SDA pin	(PB.11)
 - Check that a pull-up resistor is connected on one I2C SDA pin
 - Check that a pull-up resistor is connected on one I2C SCL pin


How to use it
=============
In order to make the program work, you must do the following :
- Create a project and setup all your toolchain's start-up files
- Compile the directory content files and required Library files :
  + stm32f10x_lib.c
  + stm32f10x_i2c.c
  + stm32f10x_rcc.c
  + stm32f10x_gpio.c
  + stm32f10x_nvic.c
  + stm32f10x_flash.c
    
- Link all compiled files and load your image into either RAM or FLASH
- Run the example


******************* (C) COPYRIGHT 2007 STMicroelectronics *****END OF FILE******
