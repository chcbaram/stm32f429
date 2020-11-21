/*
 * hw.c
 *
 *  Created on: Nov 21, 2020
 *      Author: baram
 */




#include "hw.h"



__attribute__((section(".version"))) uint8_t boot_name[32] = "F/W_B/D";
__attribute__((section(".version"))) uint8_t boot_ver[32]  = "V201120R1";



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


  uartOpen(_DEF_UART1, 57600);

  logPrintf("\n\n[ Firmware Begin... ]\r\n");
  logPrintf("Booting..Name \t\t: %s\r\n", boot_name);
  logPrintf("Booting..Ver  \t\t: %s\r\n", boot_ver);


  flashInit();

  if (sdInit() == true)
  {
    fatfsInit();
  }


  logPrintf("usb mode   \t\t: USB_CDC\r\n");
  usbBegin(USB_CDC_MODE);
  vcpInit();
}
