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
extern const struct fal_flash_dev eeprom;

/* flash device table */
#define FAL_FLASH_DEV_TABLE  \
    {                        \
        &stm32_onchip_flash, \
        &nor_flash0,         \
        &eeprom,             \
    }

/* ====================== Partition Configuration ========================== */
#define FAL_BOOT_PART_NAME "bl"
#define FAL_BOOT_START_ADDR (0 * 1024)
#define FAL_BOOT_SIZE (512 * 1024)

#define FAL_APP1_PART_NAME "app1"
#define FAL_APP1_START_ADDR (512 * 1024)
#define FAL_APP1_PART_SIZE (256 * 1024)

#define FAL_APP2_PART_NAME "app2"
#define FAL_APP2_START_ADDR (768 * 1024)
#define FAL_APP2_PART_SIZE (256 * 1024)

#define FAL_LFS_PART_NAME "littlefs"
#define FAL_LFS_START_ADDR (0 * 1024 * 1024)
#define FAL_LFS_PART_SIZE (10 * 1024 * 1024)

#define FAL_FONT_PART_NAME "font"
#define FAL_FONT_START_ADDR (10 * 1024 * 1024)
#define FAL_FONT_PART_SIZE (4 * 1024 * 1024)

#define FAL_DW_PART_NAME "download"
#define FAL_DW_START_ADDR (14 * 1024 * 1024)
#define FAL_DW_PART_SIZE (1 * 1024 * 1024)

#define FAL_NEW_PART_NAME "new"
#define FAL_NEW_START_ADDR (15 * 1024 * 1024)
#define FAL_NEW_PART_SIZE (1 * 1024 * 1024)

#define FAL_PARAM_PART_NAME "param"
#define FAL_PARAM_START_ADDR (0)
#define FAL_PARAM_PART_SIZE (256)

#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                                                         \
    {                                                                                                          \
        {FAL_PART_MAGIC_WORD, FAL_BOOT_PART_NAME, "stm32_onchip", FAL_BOOT_START_ADDR, FAL_BOOT_SIZE, 0},      \
        {FAL_PART_MAGIC_WORD, FAL_APP1_PART_NAME, "stm32_onchip", FAL_APP1_START_ADDR, FAL_APP1_PART_SIZE, 0}, \
        {FAL_PART_MAGIC_WORD, FAL_APP2_PART_NAME, "stm32_onchip", FAL_APP2_START_ADDR, FAL_APP2_PART_SIZE, 0}, \
        {FAL_PART_MAGIC_WORD, FAL_LFS_PART_NAME, "norflash0", FAL_LFS_START_ADDR, FAL_LFS_PART_SIZE, 0},       \
        {FAL_PART_MAGIC_WORD, FAL_FONT_PART_NAME, "norflash0", FAL_FONT_START_ADDR, FAL_FONT_PART_SIZE, 0},    \
        {FAL_PART_MAGIC_WORD, FAL_DW_PART_NAME, "norflash0", FAL_DW_START_ADDR, FAL_DW_PART_SIZE, 0},          \
        {FAL_PART_MAGIC_WORD, FAL_NEW_PART_NAME, "norflash0", FAL_NEW_START_ADDR, FAL_NEW_PART_SIZE, 0},       \
        {FAL_PART_MAGIC_WORD, FAL_PARAM_PART_NAME, "eeprom", FAL_PARAM_START_ADDR, FAL_PARAM_PART_SIZE, 0},    \
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
