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
#define FAL_BOOT_PART_NAME "bl"
#define FAL_BOOT_START_ADDR (0 * 1024)
#define FAL_BOOT_SIZE (512 * 1024)

#define FAL_APP1_PART_NAME "app1"
#define FAL_APP1_START_ADDR (512 * 1024) // 0x08080000
#define FAL_APP1_PART_SIZE (128 * 1024)

#define FAL_APP2_PART_NAME "app2"
#define FAL_APP2_START_ADDR (640 * 1024) // 0x080a0000
#define FAL_APP2_PART_SIZE (128 * 1024)

#define FAL_FDB_PART_NAME "fdb"
#define FAL_FDB_START_ADDR (768 * 1024) // 0x080c0000
#define FAL_FDB_PART_SIZE (256 * 1024)

#define FAL_FAC_BL_PART_NAME "factory_bl"
#define FAL_FAC_BL_START_ADDR (0 * 1024 * 1024)
#define FAL_FAC_BL_PART_SIZE (1 * 1024 * 1024)

#define FAL_FAC_APP1_PART_NAME "factory_app1"
#define FAL_FAC_APP1_START_ADDR (1 * 1024 * 1024)
#define FAL_FAC_APP1_PART_SIZE (1 * 1024 * 1024)

#define FAL_FAC_APP2_PART_NAME "factory_app2"
#define FAL_FAC_APP2_START_ADDR (2 * 1024 * 1024)
#define FAL_FAC_APP2_PART_SIZE (1 * 1024 * 1024)

#define FAL_FONT_PART_NAME "font"
#define FAL_FONT_START_ADDR (3 * 1024 * 1024)
#define FAL_FONT_PART_SIZE (5 * 1024 * 1024)

#define FAL_LFS_PART_NAME "littlefs"
#define FAL_LFS_START_ADDR (8 * 1024 * 1024)
#define FAL_LFS_PART_SIZE (8 * 1024 * 1024)

#ifdef FAL_PART_HAS_TABLE_CFG
/* partition table */
#define FAL_PART_TABLE                                                                                                  \
    {                                                                                                                   \
        {FAL_PART_MAGIC_WORD, FAL_BOOT_PART_NAME, "stm32_onchip", FAL_BOOT_START_ADDR, FAL_BOOT_SIZE, 0},               \
        {FAL_PART_MAGIC_WORD, FAL_APP1_PART_NAME, "stm32_onchip", FAL_APP1_START_ADDR, FAL_APP1_PART_SIZE, 0},          \
        {FAL_PART_MAGIC_WORD, FAL_APP2_PART_NAME, "stm32_onchip", FAL_APP2_START_ADDR, FAL_APP2_PART_SIZE, 0},          \
        {FAL_PART_MAGIC_WORD, FAL_FDB_PART_NAME, "stm32_onchip", FAL_FDB_START_ADDR, FAL_FDB_PART_SIZE, 0},             \
        {FAL_PART_MAGIC_WORD, FAL_FAC_BL_PART_NAME, "norflash0", FAL_FAC_BL_START_ADDR, FAL_FAC_BL_PART_SIZE, 0},       \
        {FAL_PART_MAGIC_WORD, FAL_FAC_APP1_PART_NAME, "norflash0", FAL_FAC_APP1_START_ADDR, FAL_FAC_APP1_PART_SIZE, 0}, \
        {FAL_PART_MAGIC_WORD, FAL_FAC_APP2_PART_NAME, "norflash0", FAL_FAC_APP2_START_ADDR, FAL_FAC_APP2_PART_SIZE, 0}, \
        {FAL_PART_MAGIC_WORD, FAL_FONT_PART_NAME, "norflash0", FAL_FONT_START_ADDR, FAL_FONT_PART_SIZE, 0},             \
        {FAL_PART_MAGIC_WORD, FAL_LFS_PART_NAME, "norflash0", FAL_LFS_START_ADDR, FAL_LFS_PART_SIZE, 0},                \
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
