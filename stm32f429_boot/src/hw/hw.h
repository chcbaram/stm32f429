/*
 * hw.h
 *
 *  Created on: Nov 21, 2020
 *      Author: baram
 */

#ifndef SRC_HW_HW_H_
#define SRC_HW_HW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "hw_def.h"

#include "led.h"
#include "uart.h"
#include "cmdif.h"
#include "button.h"
#include "swtimer.h"
#include "usb.h"
#include "flash.h"
#include "gpio.h"
#include "sd.h"
#include "fatfs/fatfs.h"


void hwInit(void);



#ifdef __cplusplus
}
#endif



#endif /* SRC_HW_HW_H_ */
