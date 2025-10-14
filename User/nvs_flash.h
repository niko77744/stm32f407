#ifndef __NVS_FLASH_H__
#define __NVS_FLASH_H__

#include "main.h"

/* FLASH起始地址 */
#define STM32_FLASH_SIZE (1024 * 1024) /* STM32 FLASH 总大小 */
#define STM32_FLASH_BASE 0x08000000    /* STM32 FLASH 起始地址 */
#define FLASH_WAITETIME 50000          /* FLASH等待超时时间 */

#define CHIP_FLASH 0
#define W25Qxx_FLASH 1

#define FLASH_TYPE CHIP_FLASH

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
void stmflash_write(uint32_t waddr, uint32_t *pbuf, uint32_t length);
void stmflash_read(uint32_t raddr, uint32_t *pbuf, uint32_t length);
void stmflash_earse(uint32_t waddr, uint32_t length);

#endif /* __NVS_FLASH_H__ */
