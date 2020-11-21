/*
 * ap.cpp
 *
 *  Created on: Nov 21, 2020
 *      Author: baram
 */




#include "ap.h"


void bootCmdif(void);
void bootJumpToFw(uint32_t addr);


void apInit(void)
{
  hwInit();
  cmdifOpen(_DEF_UART1, 57600);

  uartOpen(_DEF_UART2, 57600);

  cmdifAdd("boot", bootCmdif);
}

void apMain(void)
{
  uint32_t pre_time[2];

  pre_time[0] = millis();
  pre_time[1] = millis();
  while(1)
  {
    cmdifMain();

    if (millis()-pre_time[0] >= 500)
    {
      pre_time[0] = millis();
      ledToggle(_DEF_LED1);
    }
  }
}

void bootCmdif(void)
{

  if (cmdifGetParamCnt() == 2 && cmdifHasString("jump", 0) == true)
  {
    cmdifPrintf( "jump to fw\n");

    char *file_name;
    int file_len;
    FIL file;
    UINT len;
    FRESULT res;
    uint32_t pre_time;
    uint32_t addr_run = sdramGetAddr();


    file_name = cmdifGetParamStr(1);

    res = f_open(&file, file_name, FA_OPEN_EXISTING | FA_READ);
    if (res == FR_OK)
    {
      file_len = f_size(&file);

      pre_time = millis();
      f_read(&file, (void *)addr_run, file_len, &len);
      logPrintf("copy_fw   \t\t: %dms, %dKB\n", (int)(millis()-pre_time), (int)file_len/1024);
      f_close(&file);
    }
    else
    {
      logPrintf("no file\n");
    }
  }
}

void bootJumpToFw(uint32_t addr)
{
  void (**jump_func)(void) = (void (**)(void))(addr + 4);


  bspDeInit();

  __set_CONTROL(0x00);
  __set_MSP(*(__IO uint32_t*)addr);
  SCB->VTOR = addr;

  (*jump_func)();
}
