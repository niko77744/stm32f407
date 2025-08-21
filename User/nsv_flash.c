#include "nsv_flash.h"
#include "easyflash.h"

/* 主存储器块，分为 4 个 16 KB 扇区、1 个 64 KB 扇区和 7 个 128 KB 扇区
 块 名称 块基址 大小 主存储器
 扇区0  0x08000000 - 0x08003FFF 16KB
 扇区1  0x08004000 - 0x08007FFF 16KB
 扇区2  0x08008000 - 0x0800BFFF 16KB
 扇区3  0x0800C000 - 0x0800FFFF 16KB
 扇区4  0x08010000 - 0x0801FFFF 64KB
 扇区5  0x08020000 - 0x0803FFFF 128KB
 扇区6  0x08040000 - 0x0805FFFF 128KB
 ...
 扇区11 0x080E0000 - 0x080FFFFF 128KB */

void nvs_flash_init(void)
{
}

#define ADDR_FLASH_SECTOR_0 ((uint32_t)0x08000000)  /* Base address of Sector 0, 16 K bytes   */
#define ADDR_FLASH_SECTOR_1 ((uint32_t)0x08004000)  /* Base address of Sector 1, 16 K bytes   */
#define ADDR_FLASH_SECTOR_2 ((uint32_t)0x08008000)  /* Base address of Sector 2, 16 K bytes   */
#define ADDR_FLASH_SECTOR_3 ((uint32_t)0x0800C000)  /* Base address of Sector 3, 16 K bytes   */
#define ADDR_FLASH_SECTOR_4 ((uint32_t)0x08010000)  /* Base address of Sector 4, 64 K bytes   */
#define ADDR_FLASH_SECTOR_5 ((uint32_t)0x08020000)  /* Base address of Sector 5, 128 K bytes  */
#define ADDR_FLASH_SECTOR_6 ((uint32_t)0x08040000)  /* Base address of Sector 6, 128 K bytes  */
#define ADDR_FLASH_SECTOR_7 ((uint32_t)0x08060000)  /* Base address of Sector 7, 128 K bytes  */
#define ADDR_FLASH_SECTOR_8 ((uint32_t)0x08080000)  /* Base address of Sector 8, 128 K bytes  */
#define ADDR_FLASH_SECTOR_9 ((uint32_t)0x080A0000)  /* Base address of Sector 9, 128 K bytes  */
#define ADDR_FLASH_SECTOR_10 ((uint32_t)0x080C0000) /* Base address of Sector 10, 128 K bytes */
#define ADDR_FLASH_SECTOR_11 ((uint32_t)0x080E0000) /* Base address of Sector 11, 128 K bytes */

uint32_t stm32_get_sector(uint32_t address)
{
    uint32_t sector = 0;

    if ((address < ADDR_FLASH_SECTOR_1) && (address >= ADDR_FLASH_SECTOR_0))
        sector = FLASH_SECTOR_0;
    else if ((address < ADDR_FLASH_SECTOR_2) && (address >= ADDR_FLASH_SECTOR_1))
        sector = FLASH_SECTOR_1;
    else if ((address < ADDR_FLASH_SECTOR_3) && (address >= ADDR_FLASH_SECTOR_2))
        sector = FLASH_SECTOR_2;
    else if ((address < ADDR_FLASH_SECTOR_4) && (address >= ADDR_FLASH_SECTOR_3))
        sector = FLASH_SECTOR_3;
    else if ((address < ADDR_FLASH_SECTOR_5) && (address >= ADDR_FLASH_SECTOR_4))
        sector = FLASH_SECTOR_4;
    else if ((address < ADDR_FLASH_SECTOR_6) && (address >= ADDR_FLASH_SECTOR_5))
        sector = FLASH_SECTOR_5;
    else if ((address < ADDR_FLASH_SECTOR_7) && (address >= ADDR_FLASH_SECTOR_6))
        sector = FLASH_SECTOR_6;
    else if ((address < ADDR_FLASH_SECTOR_8) && (address >= ADDR_FLASH_SECTOR_7))
        sector = FLASH_SECTOR_7;
    else if ((address < ADDR_FLASH_SECTOR_9) && (address >= ADDR_FLASH_SECTOR_8))
        sector = FLASH_SECTOR_8;
    else if ((address < ADDR_FLASH_SECTOR_10) && (address >= ADDR_FLASH_SECTOR_9))
        sector = FLASH_SECTOR_9;
    else if ((address < ADDR_FLASH_SECTOR_11) && (address >= ADDR_FLASH_SECTOR_10))
        sector = FLASH_SECTOR_10;
    else
        sector = FLASH_SECTOR_11;

    return sector;
}

uint32_t stm32_get_sector_size(uint32_t sector)
{
    EF_ASSERT(IS_FLASH_SECTOR(sector));
    switch (sector)
    {
    case FLASH_SECTOR_0:
        return 16 * 1024;
    case FLASH_SECTOR_1:
        return 16 * 1024;
    case FLASH_SECTOR_2:
        return 16 * 1024;
    case FLASH_SECTOR_3:
        return 16 * 1024;
    case FLASH_SECTOR_4:
        return 64 * 1024;
    case FLASH_SECTOR_5:
        return 128 * 1024;
    case FLASH_SECTOR_6:
        return 128 * 1024;
    case FLASH_SECTOR_7:
        return 128 * 1024;
    case FLASH_SECTOR_8:
        return 128 * 1024;
    case FLASH_SECTOR_9:
        return 128 * 1024;
    case FLASH_SECTOR_10:
        return 128 * 1024;
    case FLASH_SECTOR_11:
        return 128 * 1024;
    default:
        return 128 * 1024;
    }
}
