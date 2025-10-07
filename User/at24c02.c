#define LOG_TAG "AT24C02"
#include "at24c02.h"
#include "elog.h"

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

at24c02_t at24c02;
extern I2C_HandleTypeDef hi2c1;
uint8_t data_buf[AT24C02_SIZE] = {0};

void at24c02_read_byte(uint8_t addr, uint8_t *data)
{
    HAL_I2C_Mem_Read(&hi2c1, at24c02.device_read_addr, addr, I2C_MEMADD_SIZE_8BIT, data, 1, 1000);
}

void at24c02_write_byte(uint8_t addr, uint8_t *data)
{
    HAL_I2C_Mem_Write(&hi2c1, at24c02.device_write_addr, addr, I2C_MEMADD_SIZE_8BIT, data, 1, 1000);
    delay_ms(AT24C02_DELAY_TIME); // 写入需要时间
}

void at24c02_erase_byte(uint8_t addr)
{
    uint8_t data = AT24C02_DEFAULT_ERASE_VALUE;
    at24c02_write_byte(addr, &data);
}

void at24c02_read(uint8_t addr, uint8_t *data, uint16_t len)
{
    HAL_I2C_Mem_Read(&hi2c1, at24c02.device_read_addr, addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

void at24c02_write_page(uint8_t addr, uint8_t *data, uint16_t len)
{
    if (len > AT24C02_PAGE_SIZE)
        len = AT24C02_PAGE_SIZE;

    HAL_I2C_Mem_Write(&hi2c1, at24c02.device_write_addr, addr, I2C_MEMADD_SIZE_8BIT, data, len, 1000);
}

void at24c02_over_write(uint8_t addr, uint8_t *data, uint16_t len)
{
    uint8_t page_remain;
    while (len > 0)
    {
        page_remain = AT24C02_PAGE_SIZE - (addr % AT24C02_PAGE_SIZE);
        if (len <= page_remain)
        {
            at24c02_write_page(addr, data, len);
            len = 0;
        }
        else
        {
            at24c02_write_page(addr, data, page_remain);
            addr += page_remain;
            data += page_remain;
            len -= page_remain;
        }
        delay_ms(AT24C02_DELAY_TIME); // 写入需要时间
    }
}

void at24c02_erase_chip(void)
{
    uint8_t page_buffer[AT24C02_PAGE_SIZE];
    uint16_t remaining = AT24C02_SIZE;
    uint8_t addr = 0;

    // 填充页缓冲区
    for (int i = 0; i < AT24C02_PAGE_SIZE; i++)
        page_buffer[i] = AT24C02_DEFAULT_ERASE_VALUE;

    while (remaining > 0)
    {
        uint8_t write_len = (remaining >= AT24C02_PAGE_SIZE) ? AT24C02_PAGE_SIZE : remaining;
        at24c02_write_page(addr, page_buffer, write_len);
        addr += write_len;
        remaining -= write_len;
        delay_ms(AT24C02_DELAY_TIME);
    }
}

// 文件系统配置
#define FS_MAGIC 0x55AA
#define FS_VERSION 1
#define MAX_FILENAME_LEN 8
#define MAX_FILE_COUNT 8
#define MAX_FILE_SIZE 32
#define AT24C02_PAGE_SIZE 16 // AT24C02的页大小

// 文件控制块
typedef struct
{
    uint16_t magic;     // 魔数
    uint8_t version;    // 版本号
    uint8_t file_count; // 文件数量
    uint8_t free_start; // 空闲空间起始地址
    uint8_t checksum;   // 头校验和
} fs_header_t;

// 文件索引项
typedef struct
{
    char name[MAX_FILENAME_LEN]; // 文件名
    uint8_t start_addr;          // 起始地址
    uint8_t size;                // 文件大小
    uint8_t flags;               // 文件标志
} file_entry_t;

// 文件系统状态
typedef struct
{
    fs_header_t header;
    file_entry_t file_table[MAX_FILE_COUNT];
    uint8_t initialized;
} tiny_fs_t;

static tiny_fs_t fs;
uint8_t fs_delete(const char *filename);
uint8_t fs_garbage_collect(void);

// CRC8校验
uint8_t fs_crc8(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x80)
            {
                crc = (crc << 1) ^ 0x07;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// 计算文件头校验和
uint8_t fs_header_checksum(fs_header_t *header)
{
    uint8_t checksum_data[5] = {
        header->magic & 0xFF,
        (header->magic >> 8) & 0xFF,
        header->version,
        header->file_count,
        header->free_start};
    return fs_crc8(checksum_data, 5);
}

// 初始化文件系统
uint8_t fs_init(void)
{
    // 读取文件系统头
    uint8_t header_buf[sizeof(fs_header_t)];
    at24c02_read(0, header_buf, sizeof(fs_header_t));
    memcpy(&fs.header, header_buf, sizeof(fs_header_t));

    // 检查魔数和校验和
    if (fs.header.magic == FS_MAGIC &&
        fs.header.checksum == fs_header_checksum(&fs.header))
    {

        // 读取文件表
        for (int i = 0; i < fs.header.file_count; i++)
        {
            uint8_t entry_buf[sizeof(file_entry_t)];
            at24c02_read(sizeof(fs_header_t) + i * sizeof(file_entry_t),
                         entry_buf, sizeof(file_entry_t));
            memcpy(&fs.file_table[i], entry_buf, sizeof(file_entry_t));
        }
        fs.initialized = 1;
        return 1; // 文件系统已存在
    }
    else
    {
        // 初始化新文件系统
        fs.header.magic = FS_MAGIC;
        fs.header.version = FS_VERSION;
        fs.header.file_count = 0;
        fs.header.free_start = sizeof(fs_header_t) + MAX_FILE_COUNT * sizeof(file_entry_t);
        fs.header.checksum = fs_header_checksum(&fs.header);

        // 写入文件系统头
        at24c02_over_write(0, (uint8_t *)&fs.header, sizeof(fs_header_t));

        fs.initialized = 1;
        return 0; // 新创建的文件系统
    }
}

// 查找文件索引
int8_t fs_find_file(const char *filename)
{
    if (!fs.initialized)
        return -1;

    for (int i = 0; i < fs.header.file_count; i++)
    {
        if (strcmp(fs.file_table[i].name, filename) == 0 && !(fs.file_table[i].flags & 0x80))
        {
            return i;
        }
    }
    return -1;
}

// 创建文件
uint8_t fs_create(const char *filename, uint8_t *data, uint8_t size)
{
    if (!fs.initialized || fs.header.file_count >= MAX_FILE_COUNT)
        return 0;

    if (strlen(filename) > MAX_FILENAME_LEN - 1 || size > MAX_FILE_SIZE)
        return 0;

    if (fs_find_file(filename) >= 0)
        return 0;

    if (fs.header.free_start + size > AT24C02_SIZE)
    {
        if (!fs_garbage_collect())
            return 0;
        if (fs.header.free_start + size > AT24C02_SIZE)
            return 0;
    }

    file_entry_t *entry = &fs.file_table[fs.header.file_count];
    strcpy(entry->name, filename);
    entry->start_addr = fs.header.free_start;
    entry->size = size;
    entry->flags = 0;

    // 使用 over_write 写入文件数据
    at24c02_over_write(entry->start_addr, data, size);

    // 写入文件表项
    uint16_t table_addr = sizeof(fs_header_t) + fs.header.file_count * sizeof(file_entry_t);
    at24c02_over_write(table_addr, (uint8_t *)entry, sizeof(file_entry_t));

    // 更新文件系统头
    fs.header.file_count++;
    fs.header.free_start += size;
    fs.header.checksum = fs_header_checksum(&fs.header);
    at24c02_over_write(0, (uint8_t *)&fs.header, sizeof(fs_header_t));

    return 1;
}

// 读取文件
uint8_t fs_read(const char *filename, uint8_t *buffer, uint8_t *size)
{
    if (!fs.initialized)
        return 0;

    int8_t index = fs_find_file(filename);
    if (index < 0)
        return 0;

    file_entry_t *entry = &fs.file_table[index];

    if (size)
    {
        *size = entry->size; // 返回文件大小
    }

    at24c02_read(entry->start_addr, buffer, entry->size);
    return 1; // 成功返回1
}

// 另一种读取方式：直接返回读取的字节数
uint8_t fs_read_data(const char *filename, uint8_t *buffer, uint8_t buffer_size)
{
    if (!fs.initialized)
        return 0;

    int8_t index = fs_find_file(filename);
    if (index < 0)
        return 0;

    file_entry_t *entry = &fs.file_table[index];

    uint8_t read_size = (entry->size <= buffer_size) ? entry->size : buffer_size;
    at24c02_read(entry->start_addr, buffer, read_size);

    return read_size; // 返回实际读取的字节数
}

// 更新文件
uint8_t fs_update(const char *filename, uint8_t *data, uint8_t size)
{
    if (!fs.initialized)
        return 0;

    int8_t index = fs_find_file(filename);
    if (index < 0)
        return 0;

    file_entry_t *entry = &fs.file_table[index];

    if (size != entry->size)
    {
        if (!fs_delete(filename))
            return 0;
        return fs_create(filename, data, size);
    }

    // 使用 over_write 更新文件数据
    at24c02_over_write(entry->start_addr, data, size);

    return 1;
}

// 删除文件
uint8_t fs_delete(const char *filename)
{
    if (!fs.initialized)
        return 0;

    int8_t index = fs_find_file(filename);
    if (index < 0)
        return 0;

    fs.file_table[index].flags |= 0x80;

    uint16_t table_addr = sizeof(fs_header_t) + index * sizeof(file_entry_t);
    at24c02_over_write(table_addr, (uint8_t *)&fs.file_table[index], sizeof(file_entry_t));

    return 1;
}

// 列出文件
uint8_t fs_list_files(char filenames[][MAX_FILENAME_LEN], uint8_t max_files)
{
    if (!fs.initialized)
        return 0;

    uint8_t count = 0;
    for (int i = 0; i < fs.header.file_count && count < max_files; i++)
    {
        if (!(fs.file_table[i].flags & 0x80))
        {
            strcpy(filenames[count], fs.file_table[i].name);
            count++;
        }
    }
    return count;
}

// 获取文件信息
uint8_t fs_get_info(const char *filename, uint8_t *size, uint8_t *address)
{
    if (!fs.initialized)
        return 0;

    int8_t index = fs_find_file(filename);
    if (index < 0)
        return 0;

    if (size)
        *size = fs.file_table[index].size;
    if (address)
        *address = fs.file_table[index].start_addr;
    return 1;
}

// 垃圾回收
uint8_t fs_garbage_collect(void)
{
    if (!fs.initialized)
        return 0;

    uint8_t temp_buffer[AT24C02_SIZE];
    uint16_t new_free_start = sizeof(fs_header_t) + MAX_FILE_COUNT * sizeof(file_entry_t);
    uint8_t new_file_count = 0;

    // 收集所有有效文件
    for (int i = 0; i < fs.header.file_count; i++)
    {
        if (!(fs.file_table[i].flags & 0x80))
        {
            // 读取文件数据到临时缓冲区
            at24c02_read(fs.file_table[i].start_addr,
                         &temp_buffer[new_free_start],
                         fs.file_table[i].size);

            // 更新文件索引
            fs.file_table[new_file_count] = fs.file_table[i];
            fs.file_table[new_file_count].start_addr = new_free_start;
            fs.file_table[new_file_count].flags = 0;

            new_free_start += fs.file_table[i].size;
            new_file_count++;
        }
    }

    // 擦除整个芯片
    at24c02_erase_chip();

    // 写入整理后的数据

    // 更新文件系统头
    fs.header.file_count = new_file_count;
    fs.header.free_start = new_free_start;
    fs.header.checksum = fs_header_checksum(&fs.header);

    // 写入文件系统头
    at24c02_over_write(0, (uint8_t *)&fs.header, sizeof(fs_header_t));

    // 写入文件表
    for (int i = 0; i < new_file_count; i++)
    {
        uint16_t table_addr = sizeof(fs_header_t) + i * sizeof(file_entry_t);
        at24c02_over_write(table_addr, (uint8_t *)&fs.file_table[i], sizeof(file_entry_t));
    }

    // 写入文件数据
    for (int i = 0; i < new_file_count; i++)
    {
        at24c02_over_write(fs.file_table[i].start_addr,
                           &temp_buffer[fs.file_table[i].start_addr],
                           fs.file_table[i].size);
    }

    return 1;
}

// 获取文件系统信息
void fs_get_stats(uint8_t *total_files, uint8_t *used_space, uint8_t *free_space)
{
    if (!fs.initialized)
        return;

    if (total_files)
        *total_files = fs.header.file_count;
    if (used_space)
        *used_space = fs.header.free_start -
                      (sizeof(fs_header_t) + MAX_FILE_COUNT * sizeof(file_entry_t));
    if (free_space)
        *free_space = AT24C02_SIZE - fs.header.free_start;
}

// 格式化文件系统
void fs_format(void)
{
    at24c02_erase_chip();
    fs_init();
}

// 检查文件系统完整性
uint8_t fs_check_integrity(void)
{
    if (!fs.initialized)
        return 0;

    // 检查文件系统头
    if (fs.header.magic != FS_MAGIC)
        return 0;
    if (fs.header.checksum != fs_header_checksum(&fs.header))
        return 0;

    // 检查文件表
    for (int i = 0; i < fs.header.file_count; i++)
    {
        if (fs.file_table[i].start_addr + fs.file_table[i].size > AT24C02_SIZE)
        {
            return 0; // 文件超出芯片范围
        }
    }

    return 1;
}

void at24c02_hw_init(void)
{
    at24c02.device_write_addr = AT24C02_ADDRESS & 0xFE; // 0xA0
    at24c02.device_read_addr = AT24C02_ADDRESS | 0x01;  // 0xA1
    at24c02.magic = AT24C02_DEFAULT_MAGIC;

    fs_format();

    // 创建配置文件
    uint8_t config_data[] = {
        0x01, // 启用标志
        0x64, // 阈值: 100
        0x00, // 模式: 0
        0x0A  // 超时: 10秒
    };

    if (fs_create("config", config_data, sizeof(config_data)))
    {
        log_i("配置文件创建成功");
    }

    // 创建日志文件
    char log_data[] = "系统启动完成";
    if (fs_create("boot_log", (uint8_t *)log_data, strlen(log_data)))
    {
        log_i("日志文件创建成功");
    }

    // 读取配置文件
    uint8_t read_config[32];
    uint8_t config_size;
    if (fs_read("config", read_config, &config_size))
    {
        log_i("读取配置文件: 大小=%d", config_size);
        log_i("启用标志: %d", read_config[0]);
        log_i("阈值: %d", read_config[1]);
        log_i("模式: %d", read_config[2]);
        log_i("超时: %d", read_config[3]);
    }

    // 列出所有文件
    char file_list[MAX_FILE_COUNT][MAX_FILENAME_LEN];
    uint8_t file_count = fs_list_files(file_list, MAX_FILE_COUNT);
    log_i("文件系统中的文件(%d个):", file_count);
    for (int i = 0; i < file_count; i++)
    {
        uint8_t size, addr;
        fs_get_info(file_list[i], &size, &addr);
        log_i("  %s - 大小:%d字节, 地址:0x%02X", file_list[i], size, addr);
    }

    // 显示文件系统状态
    uint8_t total_files, used_space, free_space;
    fs_get_stats(&total_files, &used_space, &free_space);
    log_i("文件系统状态:");
    log_i("总文件数: %d", total_files);
    log_i("已用空间: %d字节", used_space);
    log_i("剩余空间: %d字节", free_space);
    log_i("使用率: %.2f%%", (float)(used_space * 100.0f) / AT24C02_SIZE);

    // 更新文件
    uint8_t new_config[] = {0x01, 0x7F, 0x01, 0x14};
    if (fs_update("config", new_config, sizeof(new_config)))
    {
        log_i("配置文件更新成功");
    }

    // 删除文件
    if (fs_delete("config"))
    {
        log_i("日志文件删除成功");
    }
}
