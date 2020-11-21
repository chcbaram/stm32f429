/*
 * sdram.h
 *
 *  Created on: Nov 22, 2020
 *      Author: baram
 */

#ifndef SRC_COMMON_HW_INCLUDE_SDRAM_H_
#define SRC_COMMON_HW_INCLUDE_SDRAM_H_


#ifdef __cplusplus
 extern "C" {
#endif


#include "hw_def.h"

#ifdef _USE_HW_SDRAM



bool sdramInit(void);
bool sdramIsInit(void);
bool sdramTest(void);

uint32_t sdramGetAddr(void);
uint32_t sdramGetLength(void);

#endif

#ifdef __cplusplus
}
#endif


#endif /* SRC_COMMON_HW_INCLUDE_SDRAM_H_ */
