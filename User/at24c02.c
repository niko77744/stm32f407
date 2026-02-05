#define LOG_TAG "AT24C02"
#include "at24c02.h"
#include "elog.h"
#include "FreeRTOS.h"
#include "shell.h"

/* AT24CXX EEPROM size definitions */
#define AT24C01 127    /*!< AT24C01 size: 128 bytes */
#define AT24C02 255    /*!< AT24C02 size: 256 bytes */
#define AT24C04 511    /*!< AT24C04 size: 512 bytes */
#define AT24C08 1023   /*!< AT24C08 size: 1K bytes */
#define AT24C16 2047   /*!< AT24C16 size: 2K bytes */
#define AT24C32 4095   /*!< AT24C32 size: 4K bytes */
#define AT24C64 8191   /*!< AT24C64 size: 8K bytes */
#define AT24C128 16383 /*!< AT24C128 size: 16K bytes */
#define AT24C256 32767 /*!< AT24C256 size: 32K bytes */
#define AT24C512 65535 /*!< AT24C512 size: 64K bytes */

// 原理图接地
#define AT24C02_A0 0
#define AT24C02_A1 0
#define AT24C02_A2 0
#define AT24C02_ADDRESS (0x0A << 4 | (AT24C02_A2 << 3) | (AT24C02_A1 << 2) | (AT24C02_A0 << 1)) // 0xA0

#define AT24C02_DEFAULT_ERASE_VALUE 0xFF
#define AT24C02_SIZE 256     // 24c02容量256字节 256*8=2048位=2Kbit
#define AT24C02_PAGE_SIZE 16 // 24c02页大小16字节
#define AT24C02_DELAY_TIME 5 // 24c02写入延时5ms

typedef struct
{
    uint8_t device_write_addr;
    uint8_t device_read_addr;
} at24c02_addr_t;

at24c02_addr_t at24c02 = {
    .device_write_addr = AT24C02_ADDRESS,
    .device_read_addr = AT24C02_ADDRESS + 1,
};
extern I2C_HandleTypeDef hi2c1;

uint16_t at24c02_read(uint8_t addr, uint8_t *data, uint16_t len)
{
    HAL_I2C_Mem_Read(&hi2c1, at24c02.device_read_addr, addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
    return len;
}

void at24c02_write_page(uint8_t addr, uint8_t *data, uint16_t len)
{
    if (len > AT24C02_PAGE_SIZE)
        len = AT24C02_PAGE_SIZE;

    HAL_I2C_Mem_Write(&hi2c1, at24c02.device_write_addr, addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

uint16_t at24c02_write(uint8_t addr, uint8_t *data, uint16_t len)
{
    uint8_t page_buffer[AT24C02_PAGE_SIZE];
    uint8_t page_remain;
    uint16_t bytes_written = 0;

    // 参数检查
    if (addr >= AT24C02_SIZE || len == 0)
    {
        return 0;
    }
    if (addr + len > AT24C02_SIZE)
    {
        len = AT24C02_SIZE - addr; // 调整长度防止越界
    }

    while (len > 0)
    {
        uint8_t page_start = (addr / AT24C02_PAGE_SIZE) * AT24C02_PAGE_SIZE;
        page_remain = AT24C02_PAGE_SIZE - (addr % AT24C02_PAGE_SIZE);
        uint8_t current_write_len = (len <= page_remain) ? len : page_remain;

        // 如果需要写入的不是整页，先读取当前页内容
        if (current_write_len < AT24C02_PAGE_SIZE)
        {
            // 读取整页内容
            at24c02_read(page_start, page_buffer, AT24C02_PAGE_SIZE);
            // 修改需要更新的部分
            memcpy(page_buffer + (addr - page_start), data, current_write_len);
            // 写回整页
            at24c02_write_page(page_start, page_buffer, AT24C02_PAGE_SIZE);
        }
        else
        {
            // 整页写入，直接使用数据
            at24c02_write_page(addr, data, current_write_len);
        }

        addr += current_write_len;
        data += current_write_len;
        len -= current_write_len;
        bytes_written += current_write_len;

        vTaskDelay(AT24C02_DELAY_TIME); // 写入需要时间
    }

    return bytes_written;
}
uint16_t at24c02_erase(uint16_t addr, uint16_t len, uint8_t erase_value)
{
    uint8_t page_buffer[AT24C02_PAGE_SIZE];
    uint16_t total_len = len;
    uint16_t current_addr = addr;

    // 参数检查
    if (addr >= AT24C02_SIZE)
    {
        return 0;
    }
    if (addr + len > AT24C02_SIZE)
    {
        len = AT24C02_SIZE - addr;
    }

    while (len > 0)
    {
        uint8_t page_start = (current_addr / AT24C02_PAGE_SIZE) * AT24C02_PAGE_SIZE;
        uint8_t page_offset = current_addr % AT24C02_PAGE_SIZE;
        uint8_t bytes_in_page = AT24C02_PAGE_SIZE - page_offset;
        uint8_t write_len = (len < bytes_in_page) ? len : bytes_in_page;

        // 如果需要擦除的不是整页，需要先读取当前页内容
        if (write_len < AT24C02_PAGE_SIZE)
        {
            // 读取当前页内容
            at24c02_read(page_start, page_buffer, AT24C02_PAGE_SIZE);
            // 修改需要擦除的部分
            memset(page_buffer + page_offset, erase_value, write_len);
            // 写回整页
            at24c02_write_page(page_start, page_buffer, AT24C02_PAGE_SIZE);
        }
        else
        {
            // 整页擦除，直接写入擦除值
            memset(page_buffer, erase_value, AT24C02_PAGE_SIZE);
            at24c02_write_page(page_start, page_buffer, AT24C02_PAGE_SIZE);
        }

        current_addr += write_len;
        len -= write_len;
        vTaskDelay(AT24C02_DELAY_TIME); // 写入需要时间
    }

    return total_len;
}

void at24c02_erase_chip(void)
{
    at24c02_erase(0, AT24C02_SIZE, AT24C02_DEFAULT_ERASE_VALUE);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), eeprom_erase, at24c02_erase_chip, erase chip);
