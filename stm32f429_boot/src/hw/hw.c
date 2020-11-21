/*
 * hw.c
 *
 *  Created on: Nov 21, 2020
 *      Author: baram
 */




#include "hw.h"



__attribute__((section(".version"))) uint8_t boot_name[32] = "STM32F429_B/D";
__attribute__((section(".version"))) uint8_t boot_ver[32]  = "B201120R1";



void hwInit(void)
{
  bspInit();

  cmdifInit();
  swtimerInit();
  ledInit();
  buttonInit();
  gpioInit();
  uartInit();

  usbInit();
  usbBegin(USB_CDC_MODE);
  vcpInit();

  uartOpen(_DEF_UART1, 57600);

  logPrintf("\n\n[ Bootloader Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", boot_name);
  logPrintf("Booting..Ver  \t\t: %s\r\n", boot_ver);

  sdramInit();
  flashInit();

  if (sdInit() == true)
  {
    fatfsInit();
  }
}
