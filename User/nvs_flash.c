#include "nvs_flash.h"
#include "elog.h"
#include "shell.h"
#include "shell_port.h"
#include "fal.h"

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
    // EF_ASSERT(IS_FLASH_SECTOR(sector));
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

/**
 * @brief       获取某个地址所在的flash扇区
 * @param       addr    : lash地址
 * @retval      0~11,即addr所在的扇区
 */
uint8_t stmflash_get_flash_sector(uint32_t addr)
{
    if (addr < ADDR_FLASH_SECTOR_1)
        return FLASH_SECTOR_0;
    else if (addr < ADDR_FLASH_SECTOR_2)
        return FLASH_SECTOR_1;
    else if (addr < ADDR_FLASH_SECTOR_3)
        return FLASH_SECTOR_2;
    else if (addr < ADDR_FLASH_SECTOR_4)
        return FLASH_SECTOR_3;
    else if (addr < ADDR_FLASH_SECTOR_5)
        return FLASH_SECTOR_4;
    else if (addr < ADDR_FLASH_SECTOR_6)
        return FLASH_SECTOR_5;
    else if (addr < ADDR_FLASH_SECTOR_7)
        return FLASH_SECTOR_6;
    else if (addr < ADDR_FLASH_SECTOR_8)
        return FLASH_SECTOR_7;
    else if (addr < ADDR_FLASH_SECTOR_9)
        return FLASH_SECTOR_8;
    else if (addr < ADDR_FLASH_SECTOR_10)
        return FLASH_SECTOR_9;
    else if (addr < ADDR_FLASH_SECTOR_11)
        return FLASH_SECTOR_10;
    return FLASH_SECTOR_11;
}

/**
 * @brief       从指定地址读取一个字 (32位数据)
 * @param       faddr   : 读取地址 (此地址必须为4倍数!!)
 * @retval      读取到的数据 (32位)
 */
uint32_t stmflash_read_word(uint32_t faddr)
{
    return *(volatile uint32_t *)faddr;
}

/**
 * @brief       在FLASH 指定位置, 写入指定长度的数据(自动擦除)
 *   @note      因为STM32F4的扇区实在太大,没办法本地保存扇区数据,所以本函数写地址如果非0XFF
 *              ,那么会先擦除整个扇区且不保存扇区数据.所以写非0XFF的地址,将导致整个扇区数据丢失.
 *              建议写之前确保扇区里没有重要数据,最好是整个扇区先擦除了,然后慢慢往后写.
 *              该函数对OTP区域也有效!可以用来写OTP区!
 *              OTP区域地址范围:0X1FFF7800~0X1FFF7A0F(注意：最后16字节，用于OTP数据块锁定，别乱写！！)
 * @param       waddr   : 起始地址 (此地址必须为4的倍数!!,否则写入出错!)
 * @param       pbuf    : 数据指针
 * @param       length  : 要写入的 字(32位)数(就是要写入的32位数据的个数)
 * @retval      无
 */
void stmflash_write(uint32_t waddr, uint32_t *pbuf, uint32_t length)
{
    FLASH_EraseInitTypeDef flasheraseinit;
    HAL_StatusTypeDef FlashStatus = HAL_OK;

    uint32_t addrx = 0;
    uint32_t endaddr = 0;
    uint32_t sectorerror = 0;

    if (waddr < STM32_FLASH_BASE || waddr % 4 ||       /* 写入地址小于 STM32_FLASH_BASE, 或不是4的整数倍, 非法. */
        waddr > (STM32_FLASH_BASE + STM32_FLASH_SIZE)) /* 写入地址大于 STM32_FLASH_BASE + STM32_FLASH_SIZE, 非法. */
    {
        return;
    }

    HAL_FLASH_Unlock();       /* 解锁 */
    FLASH->ACR &= ~(1 << 10); /* FLASH擦除期间,必须禁止数据缓存!!! */

    addrx = waddr;                /* 写入的起始地址 */
    endaddr = waddr + length * 4; /* 写入的结束地址 */

    if (addrx < 0X1FFF0000) /* 只有主存储区,才需要执行擦除操作!! */
    {
        while (addrx < endaddr) /* 扫清一切障碍.(对非FFFFFFFF的地方,先擦除) */
        {
            if (stmflash_read_word(addrx) != 0XFFFFFFFF) /* 有非0XFFFFFFFF的地方,要擦除这个扇区 */
            {
                flasheraseinit.TypeErase = FLASH_TYPEERASE_SECTORS;       /* 擦除类型，扇区擦除 */
                flasheraseinit.Sector = stmflash_get_flash_sector(addrx); /* 要擦除的扇区 */
                flasheraseinit.NbSectors = 1;                             /* 一次只擦除一个扇区 */
                flasheraseinit.VoltageRange = FLASH_VOLTAGE_RANGE_3;      /* 电压范围，VCC=2.7~3.6V之间!! */

                if (HAL_FLASHEx_Erase(&flasheraseinit, &sectorerror) != HAL_OK)
                {
                    break; /* 发生错误了 */
                }
            }
            else
            {
                addrx += 4;
            }
            FLASH_WaitForLastOperation(FLASH_WAITETIME); /* 等待上次操作完成 */
        }
    }

    FlashStatus = FLASH_WaitForLastOperation(FLASH_WAITETIME); /* 等待上次操作完成 */

    if (FlashStatus == HAL_OK)
    {
        while (waddr < endaddr) /* 写数据 */
        {
            if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, waddr, *pbuf) != HAL_OK) /* 写入数据 */
            {
                break; /* 写入异常 */
            }

            waddr += 4;
            pbuf++;
        }
    }

    FLASH->ACR |= 1 << 10; /* FLASH擦除结束,开启数据fetch */

    HAL_FLASH_Lock(); /* 上锁 */
}

/**
 * @brief       从指定地址开始读出指定长度的数据
 * @param       raddr : 起始地址
 * @param       pbuf  : 数据指针
 * @param       length: 要读取的字(32)数,即4个字节的整数倍
 * @retval      无
 */
void stmflash_read(uint32_t raddr, uint32_t *pbuf, uint32_t length)
{
    uint32_t i;

    for (i = 0; i < length; i++)
    {
        pbuf[i] = stmflash_read_word(raddr); /* 读取4个字节. */
        raddr += 4;                          /* 偏移4个字节. */
    }
}

void stmflash_earse(uint32_t waddr, uint32_t length)
{
    FLASH_EraseInitTypeDef flasheraseinit;

    uint32_t addrx = 0;
    uint32_t endaddr = 0;
    uint32_t sectorerror = 0;

    if (waddr < STM32_FLASH_BASE || waddr % 4 ||       /* 写入地址小于 STM32_FLASH_BASE, 或不是4的整数倍, 非法. */
        waddr > (STM32_FLASH_BASE + STM32_FLASH_SIZE)) /* 写入地址大于 STM32_FLASH_BASE + STM32_FLASH_SIZE, 非法. */
    {
        return;
    }

    HAL_FLASH_Unlock();       /* 解锁 */
    FLASH->ACR &= ~(1 << 10); /* FLASH擦除期间,必须禁止数据缓存!!! */

    addrx = waddr;                /* 写入的起始地址 */
    endaddr = waddr + length * 4; /* 写入的结束地址 */

    if (addrx < 0X1FFF0000) /* 只有主存储区,才需要执行擦除操作!! */
    {
        while (addrx < endaddr) /* 扫清一切障碍.(对非FFFFFFFF的地方,先擦除) */
        {
            if (stmflash_read_word(addrx) != 0XFFFFFFFF) /* 有非0XFFFFFFFF的地方,要擦除这个扇区 */
            {
                flasheraseinit.TypeErase = FLASH_TYPEERASE_SECTORS;       /* 擦除类型，扇区擦除 */
                flasheraseinit.Sector = stmflash_get_flash_sector(addrx); /* 要擦除的扇区 */
                flasheraseinit.NbSectors = 1;                             /* 一次只擦除一个扇区 */
                flasheraseinit.VoltageRange = FLASH_VOLTAGE_RANGE_3;      /* 电压范围，VCC=2.7~3.6V之间!! */

                if (HAL_FLASHEx_Erase(&flasheraseinit, &sectorerror) != HAL_OK)
                {
                    break; /* 发生错误了 */
                }
            }
            else
            {
                addrx += 4;
            }
            FLASH_WaitForLastOperation(FLASH_WAITETIME); /* 等待上次操作完成 */
        }
    }

    FLASH->ACR |= 1 << 10; /* FLASH擦除结束,开启数据fetch */
    HAL_FLASH_Lock();      /* 上锁 */
}
