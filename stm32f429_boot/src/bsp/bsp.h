/*
 * bsp.h
 *
 *  Created on: Nov 21, 2020
 *      Author: baram
 */

#ifndef SRC_BSP_BSP_H_
#define SRC_BSP_BSP_H_


#ifdef __cplusplus
extern "C" {
#endif


#include "def.h"
#include "stm32f4xx_hal.h"

#define logPrintf(...)      printf(__VA_ARGS__);


void bspInit(void);
void bspDeInit(void);

void delay(uint32_t ms);
uint32_t millis(void);

void Error_Handler(void);

#ifdef __cplusplus
}
#endif
#endif /* SRC_BSP_BSP_H_ */
