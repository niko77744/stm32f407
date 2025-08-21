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

//[SFUD](../Lib_SFUD/src/sfud.c:116) 开始初始化通用串行闪存驱动程序（SFUD）V1.1.0。
//[SFUD](../Lib_SFUD/src/sfud.c:117) 您可以在 https://github.com/armink/SFUD 获取最新版本。
//[SFUD](../Lib_SFUD/src/sfud.c:883) 闪存设备制造商 ID 为 0xEF，内存类型 ID 为 0x40，容量 ID 为 0x18。
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:132) 检查 SFDP 头部正常。版本为 V1.0，NPN 为 0。
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:175) 检查 JEDEC 基本闪存参数头部正常。表 ID 为 0，版本为 V1.0，长度为 9，参数表指针为 0x000080。
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c：203) JEDEC 基本闪存参数表信息：
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c：204) MSB-LSB  3    2    1    0
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c：207) [0001] 0xFF 0xF1 0x20 0xE5
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c：207) [0002] 0x07 0xFF 0xFF 0xFF
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c：207) [0003] 0x6B 0x08 0xEB 0x44
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c：207) [0004] 0xBB 0x42 0x3B 0x08
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c：207) [0005] 0xFF 0xFF 0xFF 0xFE
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c：207) [0006] 0x00 0x00 0xFF 0xFF[SFUD](../Lib_SFUD/src/sfud_sfdp.c:207) [0007] 0xEB 0x21 0xFF 0xFF
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:207) [0008] 0x52 0x0F 0x20 0x0C
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:207) [0009] 0x00 0x00 0xD8 0x10
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:215) 整个设备支持 4KB 擦除。命令为 0x20。
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:234) 写入粒度为 64 字节或更大。
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:245) 目标闪存状态寄存器是非易失性的。
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:271) 仅支持 3 字节寻址。
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:305) 容量为 16777216 字节。
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c:312) 闪存设备支持 4KB 块擦除。命令为 0x20。
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c：312) 闪存设备支持 32KB 块擦除。命令为 0x52。
//[SFUD](../Lib_SFUD/src/sfud_sfdp.c：312) 闪存设备支持 64KB 块擦除。命令为 0xD8。
//[SFUD]发现了一颗旺宏闪存芯片。大小为 16777216 字节。
//[SFUD](../Lib_SFUD/src/sfud.c：861) 闪存设备复位成功。
//[SFUD]W25Q128BV 闪存设备初始化成功。

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
