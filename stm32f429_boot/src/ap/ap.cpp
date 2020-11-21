/*
 * ap.cpp
 *
 *  Created on: Nov 21, 2020
 *      Author: baram
 */




#include "ap.h"





void apInit(void)
{
  hwInit();
  cmdifOpen(_DEF_UART1, 57600);

  uartOpen(_DEF_UART2, 57600);
}

void apMain(void)
{
  uint32_t pre_time[2];

  pre_time[0] = millis();
  pre_time[1] = millis();
  while(1)
  {
    cmdifMain();

    if (millis()-pre_time[0] >= 100)
    {
      pre_time[0] = millis();
      ledToggle(_DEF_LED1);
    }
    if (millis()-pre_time[1] >= 500)
    {
      pre_time[1] = millis();
      ledToggle(_DEF_LED2);
    }

    if (uartAvailable(_DEF_UART2) > 0)
    {
      uartPrintf(_DEF_UART2, "RxUSB 0x%X\n", uartRead(_DEF_UART2));
    }
  }
}
