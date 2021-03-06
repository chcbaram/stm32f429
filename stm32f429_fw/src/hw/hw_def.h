/*
 * hw_def.h
 *
 *  Created on: Nov 21, 2020
 *      Author: baram
 */

#ifndef SRC_HW_HW_DEF_H_
#define SRC_HW_HW_DEF_H_


#include "def.h"
#include "bsp.h"


#define _USE_HW_FLASH
#define _USE_HW_SD
#define _USE_HW_FATFS
#define _USE_HW_FILES
#define _USE_HW_SDRAM



#define _USE_HW_LED
#define      HW_LED_MAX_CH          2

#define _USE_HW_UART
#define      HW_UART_MAX_CH         2

#define _USE_HW_CMDIF
#define      HW_CMDIF_LIST_MAX              32
#define      HW_CMDIF_CMD_STR_MAX           16
#define      HW_CMDIF_CMD_BUF_LENGTH        128

#define _USE_HW_BUTTON
#define      HW_BUTTON_MAX_CH       1

#define _USE_HW_SWTIMER
#define      HW_SWTIMER_MAX_CH      8

#define _USE_HW_VCP
#define _USE_HW_USB
#define      HW_USE_CDC             1
#define      HW_USE_MSC             1

#define _USE_HW_GPIO
#define      HW_GPIO_MAX_CH         6




#define _PIN_GPIO_SDCARD_DETECT     0



#define SDRAM_ADDR_IMAGE              0xD0000000    // 2MB
#define SDRAM_ADDR_FW                 0xD0200000    // 2MB
#define SDRAM_ADDR_BUF                0xD0400000    // 2MB



#endif /* SRC_HW_HW_DEF_H_ */
