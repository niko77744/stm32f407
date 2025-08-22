#include "nvs_flash.h"
#include "easyflash.h"
#include "elog.h"

// ��ֲ: https://zhuanlan.zhihu.com/p/136168426
// EasyFlash V4.0 ENV ���������ʵ��: https://mculover666.blog.csdn.net/article/details/105715982

/* ���洢���飬��Ϊ 4 �� 16 KB ������1 �� 64 KB ������ 7 �� 128 KB ����
 �� ���� ���ַ ��С ���洢��
 ����0  0x08000000 - 0x08003FFF 16KB
 ����1  0x08004000 - 0x08007FFF 16KB
 ����2  0x08008000 - 0x0800BFFF 16KB
 ����3  0x0800C000 - 0x0800FFFF 16KB
 ����4  0x08010000 - 0x0801FFFF 64KB
 ����5  0x08020000 - 0x0803FFFF 128KB
 ����6  0x08040000 - 0x0805FFFF 128KB
 ...
 ����11 0x080E0000 - 0x080FFFFF 128KB */

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
 * @brief ��ȡ blob ���ͻ������� size_t ef_get_env_blob(const char *key, void *value_buf, size_t buf_len, size_t *save_value_len);
 *
 * @param key ������������
 * @param value_buf ��Ż��������Ļ�����
 * @param buf_len �û������Ĵ�С
 * @param save_value_len ���ظû�������ʵ�ʴ洢�� flash �еĴ�С
 * @return size_t �ɹ�������������е����ݳ���
 */

/**
 * @brief ���� blob ���ͻ������� EfErrCode ef_set_env_blob(const char *key, const void *value_buf, size_t buf_len);
 *  ���� ���������������в����ڸ����ƵĻ�������ʱ�����ִ������������
 *  �޸� ������еĻ������������ڵ�ǰ�����������д��ڣ���Ѹû�������ֵ�޸�Ϊ����е�ֵ��
 *  ɾ����������е�valueΪNULLʱ�����ɾ���������Ӧ�Ļ���������
 *
 * @param key ������������
 * @param value_buf ��������ֵ������
 * @param buf_len ���������ȣ���ֵ�ĳ���
 * @return EfErrCode
 */

/**
 * Env demo.
 */
void test_env(void)
{
    uint32_t i_boot_times = NULL;
    char *c_old_boot_times, c_new_boot_times[11] = {0};

    /*���½ӿ��� V4.0 ����Ȼ���ã����Ѿ���������ԭ�򱻷��������ܽ����� V5.0 �汾�б���ʽɾ�� ���� V4.0 �汾��ʼ���ڸú����ڲ����л��������Ļ��������������������ͬʱʹ�øú������������´��룺
    // �����ʹ�÷���
    ssid = ef_get_env("ssid");
    password = ef_get_env("password"); // ���� buf ���ã�password �� ssid �᷵����ͬ�� buf ��ַ

    // �����Ϊ����ķ�ʽ
    ssid = strdup(ef_get_env("ssid")); // ��¡��ȡ�����Ļ�������
    password = strdup(ef_get_env("password"));

    // ʹ����ɺ��ͷ���Դ
    free(ssid); // �� strdup �ɶ�
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
    // /* ��ȡĬ�ϻ�������ֵ */
    // // ������������δ֪���Ȼ�ȡ Flash �ϴ洢��ʵ�ʳ��� */
    // ef_get_env_blob("wifi_ssid", NULL, 0, &len);
    // // ��ȡ��������
    // ef_get_env_blob("wifi_ssid", wifi_ssid, len, NULL);
    // // ��ӡ��ȡ�Ļ�������ֵ
    // log_i("wifi_ssid env is:%s\r\n", wifi_ssid);
    // // ������������δ֪���Ȼ�ȡ Flash �ϴ洢��ʵ�ʳ��� */
    // ef_get_env_blob("wifi_passwd", NULL, 0, &len);
    // // ��ȡ��������
    // ef_get_env_blob("wifi_passwd", wifi_passwd, len, NULL);
    // // ��ӡ��ȡ�Ļ�������ֵ
    // log_i("wifi_passwd env is:%s\r\n", wifi_passwd);
    // /* ����������ֵ�ı� */
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
