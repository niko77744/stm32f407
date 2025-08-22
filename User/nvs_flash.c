#include "nvs_flash.h"
#include "easyflash.h"
#include "elog.h"

// 移植: https://zhuanlan.zhihu.com/p/136168426
// EasyFlash V4.0 ENV 功能设计与实现: https://mculover666.blog.csdn.net/article/details/105715982

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

/**
 * @brief 获取 blob 类型环境变量 size_t ef_get_env_blob(const char *key, void *value_buf, size_t buf_len, size_t *save_value_len);
 *
 * @param key 环境变量名称
 * @param value_buf 存放环境变量的缓冲区
 * @param buf_len 该缓冲区的大小
 * @param save_value_len 返回该环境变量实际存储在 flash 中的大小
 * @return size_t 成功存放至缓冲区中的数据长度
 */

/**
 * @brief 设置 blob 类型环境变量 EfErrCode ef_set_env_blob(const char *key, const void *value_buf, size_t buf_len);
 *  增加 ：当环境变量表中不存在该名称的环境变量时，则会执行新增操作；
 *  修改 ：入参中的环境变量名称在当前环境变量表中存在，则把该环境变量值修改为入参中的值；
 *  删除：当入参中的value为NULL时，则会删除入参名对应的环境变量。
 *
 * @param key 环境变量名称
 * @param value_buf 环境变量值缓冲区
 * @param buf_len 缓冲区长度，即值的长度
 * @return EfErrCode
 */

/**
 * Env demo.
 */
void test_env(void)
{
    uint32_t i_boot_times = NULL;
    char *c_old_boot_times, c_new_boot_times[11] = {0};

    /*以下接口在 V4.0 中仍然可用，但已经由于种种原因被废弃，可能将会在 V5.0 版本中被正式删除 由于 V4.0 版本开始，在该函数内部具有环境变量的缓冲区，不允许连续多次同时使用该函数，例如如下代码：
    // 错误的使用方法
    ssid = ef_get_env("ssid");
    password = ef_get_env("password"); // 由于 buf 共用，password 与 ssid 会返回相同的 buf 地址

    // 建议改为下面的方式
    ssid = strdup(ef_get_env("ssid")); // 克隆获取回来的环境变量
    password = strdup(ef_get_env("password"));

    // 使用完成后，释放资源
    free(ssid); // 与 strdup 成对
    free(password);*/

    /* get the boot count number from Env */
    c_old_boot_times = ef_get_env("boot_times");
    i_boot_times = atol(c_old_boot_times);
    /* boot count +1 */
    i_boot_times++;
    log_i("The system now boot %d times\n", i_boot_times);
    /* interger to string */
    sprintf(c_new_boot_times, "%u", i_boot_times);
    /* set and store the boot count number to Env */
    ef_set_env("boot_times", c_new_boot_times);
    ef_save_env();

    // char wifi_ssid[20] = {0};
    // char wifi_passwd[20] = {0};
    // size_t len = 0;
    // /* 读取默认环境变量值 */
    // // 环境变量长度未知，先获取 Flash 上存储的实际长度 */
    // ef_get_env_blob("wifi_ssid", NULL, 0, &len);
    // // 获取环境变量
    // ef_get_env_blob("wifi_ssid", wifi_ssid, len, NULL);
    // // 打印获取的环境变量值
    // log_i("wifi_ssid env is:%s\r\n", wifi_ssid);
    // // 环境变量长度未知，先获取 Flash 上存储的实际长度 */
    // ef_get_env_blob("wifi_passwd", NULL, 0, &len);
    // // 获取环境变量
    // ef_get_env_blob("wifi_passwd", wifi_passwd, len, NULL);
    // // 打印获取的环境变量值
    // log_i("wifi_passwd env is:%s\r\n", wifi_passwd);
    // /* 将环境变量值改变 */
    // ef_set_env_blob("wifi_ssid", "SSID_TEST", 9);
    // ef_set_env_blob("wifi_passwd", "66666666", 8);
}

void nvs_flash_init(void)
{
    if (easyflash_init() == EF_NO_ERR)
    {
        /* test Env demo */
        test_env();
    }
}
