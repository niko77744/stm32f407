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

/* EEPROM type selection - Using AT24C512, modify EE_TYPE for other types */
#define EE_TYPE AT24C02

// 原理图接地
#define AT24C02_A0 0
#define AT24C02_A1 0
#define AT24C02_A2 0
#define AT24C02_ADDRESS (0x0A << 4 | (AT24C02_A2 << 3) | (AT24C02_A1 << 2) | (AT24C02_A0 << 1)) // 0xA0

#define AT24C02_DEFAULT_ERASE_VALUE 0xFF
#define AT24C02_DEFAULT_MAGIC 0x55
#define AT24C02_SIZE 256     // 24c02容量256字节 256*8=2048位=2Kbit
#define AT24C02_PAGE_SIZE 16 // 24c02页大小16字节
#define AT24C02_DELAY_TIME 5 // 24c02写入延时5ms

typedef struct
{
    uint8_t device_write_addr;
    uint8_t device_read_addr;
    uint8_t magic;
    uint8_t data[AT24C02_SIZE];
} at24c02_t;

at24c02_t at24c02 = {
    .device_write_addr = AT24C02_ADDRESS,
    .device_read_addr = AT24C02_ADDRESS + 1,
    .magic = AT24C02_DEFAULT_MAGIC,
};
extern I2C_HandleTypeDef hi2c1;
uint8_t data_buf[AT24C02_SIZE] = {0};

void at24c02_read_byte(uint8_t addr, uint8_t *data)
{
    HAL_I2C_Mem_Read(&hi2c1, at24c02.device_read_addr, addr, I2C_MEMADD_SIZE_8BIT, data, 1, 1000);
}

void at24c02_write_byte(uint8_t addr, uint8_t *data)
{
    HAL_I2C_Mem_Write(&hi2c1, at24c02.device_write_addr, addr, I2C_MEMADD_SIZE_8BIT, data, 1, 1000);
    vTaskDelay(AT24C02_DELAY_TIME); // 写入需要时间
}

void at24c02_erase_byte(uint8_t addr)
{
    uint8_t data = AT24C02_DEFAULT_ERASE_VALUE;
    at24c02_write_byte(addr, &data);
}

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

static void eeprom(uint8_t argc, char **argv)
{

#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
#define HEXDUMP_WIDTH 16
#define CMD_PROBE_INDEX 0
#define CMD_READ_INDEX 1
#define CMD_WRITE_INDEX 2
#define CMD_ERASE_INDEX 3
#define CMD_BENCH_INDEX 4

    int result = 0;
    size_t i = 0, j = 0;

    const char *help_info[] =
        {
            [CMD_PROBE_INDEX] = "fal probe [dev_name|part_name]   - probe flash device or partition by given name",
            [CMD_READ_INDEX] = "fal read addr size               - read 'size' bytes starting at 'addr'",
            [CMD_WRITE_INDEX] = "fal write addr data1 ... dataN   - write some bytes 'data' starting at 'addr'",
            [CMD_ERASE_INDEX] = "fal erase addr size              - erase 'size' bytes starting at 'addr'",
            [CMD_BENCH_INDEX] = "fal bench <blk_size>             - benchmark test with per block size",
        };

    if (argc < 2)
    {
        printf("Usage:\n");
        for (i = 0; i < sizeof(help_info) / sizeof(char *); i++)
        {
            printf("%s\n", help_info[i]);
        }
        printf("\n");
    }
    else
    {
        const char *operator = argv[1];
        uint32_t addr = 0, size = 0;

        if (!strcmp(operator, "read"))
        {
            if (argc < 4)
            {
                printf("Usage: %s.\n", help_info[CMD_READ_INDEX]);
                return;
            }
            else
            {
                addr = strtol(argv[2], NULL, 0);
                size = strtol(argv[3], NULL, 0);
                uint8_t *data = malloc(size);
                if (data)
                {
                    result = at24c02_read(addr, data, size);
                    if (result >= 0)
                    {
                        printf("Read data success. Start from 0x%08X, size is %ld. The data is:\n", addr, size);
                        printf("Offset (h) 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
                        for (i = 0; i < size; i += HEXDUMP_WIDTH)
                        {
                            printf("[%08X] ", addr + i);
                            /* dump hex */
                            for (j = 0; j < HEXDUMP_WIDTH; j++)
                            {
                                if (i + j < size)
                                {
                                    printf("%02X ", data[i + j]);
                                }
                                else
                                {
                                    printf("   ");
                                }
                            }
                            /* dump char for hex */
                            for (j = 0; j < HEXDUMP_WIDTH; j++)
                            {
                                if (i + j < size)
                                {
                                    printf("%c", __is_print(data[i + j]) ? data[i + j] : '.');
                                }
                            }
                            printf("\n");
                        }
                        printf("\n");
                    }
                    free(data);
                }
                else
                {
                    printf("Low memory!\n");
                }
            }
        }
        else if (!strcmp(operator, "write"))
        {
            if (argc < 4)
            {
                printf("Usage: %s.\n", help_info[CMD_WRITE_INDEX]);
                return;
            }
            else
            {
                addr = strtol(argv[2], NULL, 0);
                size = argc - 3;
                uint8_t *data = malloc(size);
                if (data)
                {
                    for (i = 0; i < size; i++)
                    {
                        data[i] = strtol(argv[3 + i], NULL, 0);
                    }
                    result = at24c02_write(addr, data, size);
                    if (result >= 0)
                    {
                        printf("Write data success. Start from 0x%08X, size is %ld.\n", addr, size);
                        printf("Write data: ");
                        for (i = 0; i < size; i++)
                        {
                            printf("%d ", data[i]);
                        }
                        printf(".\n");
                    }
                    free(data);
                }
                else
                {
                    printf("Low memory!\n");
                }
            }
        }
        else if (!strcmp(operator, "erase"))
        {
            if (argc < 4)
            {
                printf("Usage: %s.\n", help_info[CMD_ERASE_INDEX]);
                return;
            }
            else
            {
                addr = strtol(argv[2], NULL, 0);
                size = strtol(argv[3], NULL, 0);
                result = at24c02_erase(addr, size, AT24C02_DEFAULT_ERASE_VALUE);
                if (result >= 0)
                {
                    printf("Erase data success. Start from 0x%08X, size is %ld.\n", addr, size);
                }
            }
        }
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), eeprom, eeprom, eeprom to probe read write erase bench);
