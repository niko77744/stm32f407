#include "w25qxx.h"

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

static void sfud_demo(uint32_t addr, uint32_t size, uint8_t *data)
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
        printf("Erase the %s flash data finish. Start from 0x%08X, size is %d.\r\n", flash->name, addr, size);
    }
    else
    {
        printf("Erase the %s flash data failed.\r\n", flash->name);
        return;
    }
    /* write test */
    result = sfud_write(flash, addr, size, data);
    if (result == SFUD_SUCCESS)
    {
        printf("Write the %s flash data finish. Start from 0x%08X, size is %d.\r\n", flash->name, addr, size);
    }
    else
    {
        printf("Write the %s flash data failed.\r\n", flash->name);
        return;
    }
    /* read test */
    result = sfud_read(flash, addr, size, data);
    if (result == SFUD_SUCCESS)
    {
        printf("Read the %s flash data success. Start from 0x%08X, size is %d. The data is:\r\n", flash->name, addr, size);
        printf("Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\r\n");
        for (i = 0; i < size; i++)
        {
            if (i % 16 == 0)
            {
                printf("[%08X] ", addr + i);
            }
            printf("%02X ", data[i]);
            if (((i + 1) % 16 == 0) || i == size - 1)
            {
                printf("\r\n");
            }
        }
        printf("\r\n");
    }
    else
    {
        printf("Read the %s flash data failed.\r\n", flash->name);
    }
    /* data check */
    for (i = 0; i < size; i++)
    {
        if (data[i] != i % 256)
        {
            printf("Read and check write data has an error. Write the %s flash data failed.\r\n", flash->name);
            break;
        }
    }
    if (i == size)
    {
        printf("The %s flash test is success.\r\n", flash->name);
    }
}

#define SFUD_BUFFER_SIZE 1024
uint8_t sfud_buf[SFUD_BUFFER_SIZE];

void sfud_w25qxx_init(void)
{
    if (sfud_init() == SFUD_SUCCESS)
    {
        sfud_demo(0, sizeof(sfud_buf), sfud_buf);
    }
}
