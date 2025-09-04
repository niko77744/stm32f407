/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef _FAL_CFG_H_
#define _FAL_CFG_H_

#define FAL_DEBUG 1
#define FAL_PART_HAS_TABLE_CFG
#define FAL_USING_SFUD_PORT

/* ===================== Flash device Configuration ========================= */
extern const struct fal_flash_dev stm32_onchip_flash;
extern struct fal_flash_dev nor_flash0;

/* flash device table */
#define FAL_FLASH_DEV_TABLE  \
    {                        \
        &stm32_onchip_flash, \
        &nor_flash0,         \
    }

/* ====================== Partition Configuration ========================== */
#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                               \
    {                                                                                \
        {FAL_PART_MAGIC_WORD, "bl", "stm32_onchip", 256 * 1024, 256 * 1024, 0},      \
        {FAL_PART_MAGIC_WORD, "app", "stm32_onchip", 512 * 1024, 256 * 1024, 0},     \
        {FAL_PART_MAGIC_WORD, "easyflash", "norflash0", 0, 1024 * 1024, 0},          \
        {FAL_PART_MAGIC_WORD, "download", "norflash0", 1024 * 1024, 1024 * 1024, 0}, \
    }
#endif /* FAL_PART_HAS_TABLE_CFG */
/*
#define FAL_PART_TABLE                                                               \
{                                                                                    \
    {FAL_PART_MAGIC_WORD,        "bl",     "stm32_onchip",         0,   64*1024, 0}, \
    {FAL_PART_MAGIC_WORD,       "app",     "stm32_onchip",   64*1024,  704*1024, 0}, \
    {FAL_PART_MAGIC_WORD, "easyflash", NOR_FLASH_DEV_NAME,         0, 1024*1024, 0}, \
    {FAL_PART_MAGIC_WORD,  "download", NOR_FLASH_DEV_NAME, 1024*1024, 1024*1024, 0}, \
}
| 分区名         | Flash 设备名    | 偏移地址   | 大小  | 说明
| "bl"           | "stm32_onchip" | 0         | 64KB  | 引导程序
| "app"          | "stm32_onchip" | 64*1024   | 704KB | 应用程序
| "easyflash"    | "norflash0"    | 0         | 1MB   | EasyFlash 参数存储
| "download"     | "norflash0"    | 1024*1024 | 1MB   | OTA 下载区
*/
#endif /* _FAL_CFG_H_ */
