/*
 * This file is part of the EasyFlash Library.
 *
 * Copyright (c) 2015-2019, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-01-16
 */
#define LOG_TAG "easy_flash"
#include <easyflash.h>
#include <stdarg.h>
#include "elog.h"
#include "main.h"
#include <sfud.h>

#pragma diag_suppress 177 // 抑制本文件中的 177 警告 -- 忽略未使用的函数警告

/* default environment variables set for user */
static const ef_env default_env_set[] = {
    {"iap_need_copy_app", "0"},
    {"iap_copy_app_size", "0"},
    {"stop_in_bootloader", "0"},
    {"device_id", "1"},
    {"boot_times", "0"},
};

static char log_buf[1024];

/**
 * Flash port for hardware initialize.
 *
 * @param default_env default ENV set for user
 * @param default_env_size default ENV size
 *
 * @return result
 */
EfErrCode ef_port_init(ef_env const **default_env, size_t *default_env_size)
{
    EfErrCode result = EF_NO_ERR;

    *default_env = default_env_set;
    *default_env_size = sizeof(default_env_set) / sizeof(default_env_set[0]);

#if FLASH_TYPE == W25Qxx_FLASH
    if (sfud_init() != SFUD_SUCCESS)
        result = EF_ENV_INIT_FAILED;
#endif
    return result;
}

/**
 * Read data from flash.
 * @note This operation's units is word.
 *
 * @param addr flash address
 * @param buf buffer to store read data
 * @param size read bytes size
 *
 * @return result
 */
EfErrCode ef_port_read(uint32_t addr, uint32_t *buf, size_t size)
{
    EfErrCode result = EF_NO_ERR;
#if FLASH_TYPE == CHIP_FLASH
    uint8_t *buf_8 = (uint8_t *)buf;
    /* You can add your code under here. */
    for (size_t i = 0; i < size; i++, addr++, buf_8++)
    {
        *buf_8 = *(uint8_t *)addr;
    }
#elif FLASH_TYPE == W25Qxx_FLASH
    const sfud_flash *flash = sfud_get_device_table() + SFUD_W25Qxx_DEVICE_INDEX;
    sfud_read(flash, addr, size, (uint8_t *)buf);
#endif
    return result;
}

/**
 * Erase data on flash.
 * @note This operation is irreversible.
 * @note This operation's units is different which on many chips.
 *
 * @param addr flash address
 * @param size erase bytes size
 *
 * @return result
 */
EfErrCode ef_port_erase(uint32_t addr, size_t size)
{
    EfErrCode result = EF_NO_ERR;
#if FLASH_TYPE == CHIP_FLASH
    FLASH_EraseInitTypeDef erase_init = {0};
    size_t erased_size = 0;
    uint32_t cur_erase_sector;
    uint32_t erase_error;
    HAL_StatusTypeDef flash_status;
    extern uint32_t stm32_get_sector(uint32_t address);
    extern uint32_t stm32_get_sector_size(uint32_t sector);
    /* make sure the start address is a multiple of EF_ERASE_MIN_SIZE */
    EF_ASSERT(addr % EF_ERASE_MIN_SIZE == 0);

    /* You can add your code under here. */
    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    while (erased_size < size)
    {
        cur_erase_sector = stm32_get_sector(addr + erased_size);

        FLASH_EraseInitTypeDef erase_init = {
            // .Banks = FLASH_BANK_1,
            // .VoltageRange = FLASH_VOLTAGE_RANGE_3,
            .TypeErase = FLASH_TYPEERASE_SECTORS,
            .Sector = cur_erase_sector,
            .NbSectors = 1,
        };

        flash_status = HAL_FLASHEx_Erase(&erase_init, &erase_error);
        if (flash_status != HAL_OK)
        {
            result = EF_ERASE_ERR;
            break;
        }
        erased_size += stm32_get_sector_size(cur_erase_sector);
    }
    HAL_FLASH_Lock();
#elif FLASH_TYPE == W25Qxx_FLASH

    sfud_err sfud_result = SFUD_SUCCESS;

    const sfud_flash *flash = sfud_get_device_table() + SFUD_W25Qxx_DEVICE_INDEX;

    /* make sure the start address is a multiple of FLASH_ERASE_MIN_SIZE */
    EF_ASSERT(addr % EF_ERASE_MIN_SIZE == 0);

    // ??SFUD???API??Flash??
    sfud_result = sfud_erase(flash, addr, size);

    if (sfud_result != SFUD_SUCCESS)
    {
        result = EF_ERASE_ERR;
    }
#endif

    return result;
}
/**
 * Write data to flash.
 * @note This operation's units is word.
 * @note This operation must after erase. @see flash_erase.
 *
 * @param addr flash address
 * @param buf the write data buffer
 * @param size write bytes size
 *
 * @return result
 */
EfErrCode ef_port_write(uint32_t addr, const uint32_t *buf, size_t size)
{
    EfErrCode result = EF_NO_ERR;
#if FLASH_TYPE == CHIP_FLASH
    /* You can add your code under here. */
    size_t i;
    uint32_t read_data;
    uint8_t *buf_8 = (uint8_t *)buf;

    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

    for (i = 0; i < size; i++, buf_8++, addr++)
    {
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, addr, *buf_8);
        /* write data */
        read_data = *(uint8_t *)addr;
        /* check data */
        if (read_data != *buf_8)
        {
            result = EF_WRITE_ERR;
            break;
        }
    }
    HAL_FLASH_Lock();

#elif FLASH_TYPE == W25Qxx_FLASH

    sfud_err sfud_result = SFUD_SUCCESS;

    const sfud_flash *flash = sfud_get_device_table() + SFUD_W25Qxx_DEVICE_INDEX;

    sfud_result = sfud_write(flash, addr, size, (const uint8_t *)buf);

    if (sfud_result != SFUD_SUCCESS)
    {
        result = EF_WRITE_ERR;
    }
#endif
    return result;
}

/**
 * lock the ENV ram cache
 */
void ef_port_env_lock(void)
{
    // 关闭全局中断
    __disable_irq();
    /* You can add your code under here. */
}

/**
 * unlock the ENV ram cache
 */
void ef_port_env_unlock(void)
{
    // 打开全局中断
    __enable_irq();
    /* You can add your code under here. */
}

/**
 * This function is print flash debug info.
 *
 * @param file the file which has call this function
 * @param line the line number which has call this function
 * @param format output format
 * @param ... args
 *
 */
void ef_log_debug(const char *file, const long line, const char *format, ...)
{
#ifdef PRINT_DEBUG
    va_list args;
    /* args point to the first variable parameter */
    va_start(args, format);
    log_d("[Flash](%s:%ld) ", file, line);
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    log_d("%s", log_buf);
    va_end(args);
#endif
}

/**
 * This function is print flash routine info.
 *
 * @param format output format
 * @param ... args
 */
void ef_log_info(const char *format, ...)
{
    va_list args;
    /* args point to the first variable parameter */
    va_start(args, format);
    log_i("[Flash]");
    /* must use vprintf to print */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    log_i("%s", log_buf);
    va_end(args);
}
/**
 * This function is print flash non-package info.
 *
 * @param format output format
 * @param ... args
 */
void ef_print(const char *format, ...)
{
    va_list args;

    /* args point to the first variable parameter */
    va_start(args, format);
    /* You can add your code under here. */
    vsnprintf(log_buf, sizeof(log_buf), format, args);
    log_i("%s", log_buf);
    va_end(args);
}
