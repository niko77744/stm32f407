#ifndef __NSV_FLASH_H__
#define __NSV_FLASH_H__

#include "main.h"

#define CHIP_FLASH 0
#define W25Qxx_FLASH 1

#define FLASH_TYPE W25Qxx_FLASH

#if FLASH_TYPE == CHIP_FLASH
#define USER_ERASE_MIN_SIEZ (128 * 1024)
#define USER_WRITE_GRAN (8)
#define USER_START_ADDR (FLASH_BASE + USER_ERASE_MIN_SIEZ) /* on the chip position: 128KB */
#elif FLASH_TYPE == W25Qxx_FLASH
#define USER_ERASE_MIN_SIEZ (4 * 1024)
#define USER_WRITE_GRAN (1) /* only support 1(nor flash)/ 8(stm32f4)/ 32(stm32f1)*/
#define USER_START_ADDR (0) /* on the chip position: 128KB */
#endif

void nvs_flash_init(void);
#endif /* __NSV_FLASH_H__ */
