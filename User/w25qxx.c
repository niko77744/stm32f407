#define LOG_TAG "w25qxx"

#include "w25qxx.h"
#include "elog.h"

w25qxx_device_t w25q32_dev = {0};

uint8_t spi1_read_write_byte(uint8_t tx_data)
{
    uint8_t rx_data = 0;
    HAL_SPI_TransmitReceive(&hspi1, &tx_data, &rx_data, 1, 1000);
    return rx_data;
}

uint16_t w25qxx_read_id(void)
{
    uint16_t id = 0;
    HAL_GPIO_WritePin(W25Qxx_CS_GPIO_Port, W25Qxx_CS_Pin, GPIO_PIN_RESET);
    spi1_read_write_byte(W25X_ManufactDeviceID);
    spi1_read_write_byte(0x00);
    spi1_read_write_byte(0x00);
    spi1_read_write_byte(0x00);
    id |= spi1_read_write_byte(0xFF) << 8;
    id |= spi1_read_write_byte(0xFF);
    HAL_GPIO_WritePin(W25Qxx_CS_GPIO_Port, W25Qxx_CS_Pin, GPIO_PIN_SET);
    return id;
}

//[SFUD](../Lib_SFUD/src/sfud.c:116) ��ʼ��ʼ��ͨ�ô���������������SFUD��V1.1.0��
//[SFUD](../Lib_SFUD/src/sfud.c:117) �������� https://github.com/armink/SFUD ��ȡ���°汾��
//[SFUD](../Lib_SFUD/src/sfud.c:883) �����豸������ ID Ϊ 0xEF���ڴ����� ID Ϊ 0x40������ ID Ϊ 0x18��
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:132) ��� SFDP ͷ���������汾Ϊ V1.0��NPN Ϊ 0��
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:175) ��� JEDEC �����������ͷ���������� ID Ϊ 0���汾Ϊ V1.0������Ϊ 9��������ָ��Ϊ 0x000080��
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c��203) JEDEC ���������������Ϣ��
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c��204) MSB-LSB  3    2    1    0
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c��207) [0001] 0xFF 0xF1 0x20 0xE5
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c��207) [0002] 0x07 0xFF 0xFF 0xFF
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c��207) [0003] 0x6B 0x08 0xEB 0x44
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c��207) [0004] 0xBB 0x42 0x3B 0x08
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c��207) [0005] 0xFF 0xFF 0xFF 0xFE
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c��207) [0006] 0x00 0x00 0xFF 0xFF[SFUD](../Lib_SFUD/src/sfud_sfdp.c:207) [0007] 0xEB 0x21 0xFF 0xFF
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:207) [0008] 0x52 0x0F 0x20 0x0C
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:207) [0009] 0x00 0x00 0xD8 0x10
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:215) �����豸֧�� 4KB ����������Ϊ 0x20��
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:234) д������Ϊ 64 �ֽڻ����
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:245) Ŀ������״̬�Ĵ����Ƿ���ʧ�Եġ�
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:271) ��֧�� 3 �ֽ�Ѱַ��
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:305) ����Ϊ 16777216 �ֽڡ�
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:312) �����豸֧�� 4KB �����������Ϊ 0x20��
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c��312) �����豸֧�� 32KB �����������Ϊ 0x52��
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c��312) �����豸֧�� 64KB �����������Ϊ 0xD8��
//[SFUD]������һ����������оƬ����СΪ 16777216 �ֽڡ�
//[SFUD](../Lib_SFUD/src/sfud.c��861) �����豸��λ�ɹ���
//[SFUD]W25Q128BV �����豸��ʼ���ɹ���

static void sfud_w25qxx_self_inspection(uint32_t addr, uint32_t size, uint8_t *data)
{
    sfud_err result = SFUD_SUCCESS;
    const sfud_flash *flash = sfud_get_device_table() + 0;
    uint32_t i;
    /* prepare write data */
    for (i = 0; i < size; i++)
    {
        data[i] = i;
    }
    /* erase test */
    result = sfud_erase(flash, addr, size);
    if (result == SFUD_SUCCESS)
    {
        log_i("Erase the %s flash data finish. Start from 0x%08X, size is %d.", flash->name, addr, size);
    }
    else
    {
        log_e("Erase the %s flash data failed.", flash->name);
        return;
    }
    /* write test */
    result = sfud_write(flash, addr, size, data);
    if (result == SFUD_SUCCESS)
    {
        log_i("Write the %s flash data finish. Start from 0x%08X, size is %d.", flash->name, addr, size);
    }
    else
    {
        log_e("Write the %s flash data failed.", flash->name);
        return;
    }
    /* read test */
    result = sfud_read(flash, addr, size, data);
    if (result == SFUD_SUCCESS)
    {
        log_i("Read the %s flash data success. Start from 0x%08X, size is %d.", flash->name, addr, size);
    }
    else
    {
        log_e("Read the %s flash data failed.", flash->name);
    }
    /* data check */
    for (i = 0; i < size; i++)
    {
        if (data[i] != i % 256)
        {
            log_e("Read and check write data has an error. Write the %s flash data failed.", flash->name);
            break;
        }
    }
    if (i == size)
    {
        log_i("The %s flash test is success.", flash->name);
    }
}

#define SFUD_BUFFER_SIZE 1024
uint8_t sfud_buf[SFUD_BUFFER_SIZE];

void sfud_w25qxx_init(void)
{
    if (sfud_init() == SFUD_SUCCESS)
    {
        sfud_w25qxx_self_inspection(0, sizeof(sfud_buf), sfud_buf);
    }
}
