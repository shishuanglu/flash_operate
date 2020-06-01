/*
 * flash.h
 *
 *  Created on: Jun 1, 2020
 *      Author: Administrator
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "typedef.h"
#include "stm32f1xx_hal.h"

#define SOFT_VER_ADDR 0x8001000
#define SOFT_VER	"batteryV2.0_20200601"
#define WRITE_SOFT_VERSION() FLASH_Write(SOFT_VER_ADDR,(U16*)SOFT_VER,sizeof(SOFT_VER))



/* FLASH大小：128K */
#define STM32FLASH_SIZE  0x00020000UL

/* FLASH起始地址 */
#define STM32FLASH_BASE  FLASH_BASE

/* FLASH结束地址 */
#define STM32FLASH_END   (STM32FLASH_BASE | STM32FLASH_SIZE)

/* FLASH页大小：1K */
#define STM32FLASH_PAGE_SIZE FLASH_PAGE_SIZE

/* FLASH总页数 */
#define STM32FLASH_PAGE_NUM  (STM32FLASH_SIZE / STM32FLASH_PAGE_SIZE)

/* 获取页地址，X=0~STM32FLASH_PAGE_NUM */
#define ADDR_FLASH_PAGE_X(X)    (STM32FLASH_BASE | (X * STM32FLASH_PAGE_SIZE))


/////////////////////////////////////////// 导出函数声明 ////////////////////////////////////////////
void FLASH_Init(void);
uint32_t FLASH_Read(uint32_t Address, uint16_t *Buffer, uint32_t NumToRead);
uint32_t FLASH_Write(uint32_t Address, const uint16_t *Buffer, uint32_t NumToWrite);


#endif /* INC_FLASH_H_ */
