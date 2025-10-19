#define LOG_TAG "w25qxx"

#include "w25qxx.h"
#include "elog.h"
#include "shell.h"

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

#if 0
/**
 * @brief 自检程序
 *
 * @param addr
 * @param size
 * @param data
 */
DEPRECATED static void sfud_w25qxx_self_inspection(uint32_t addr, uint32_t size, uint8_t *data)
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

DEPRECATED void sfud_w25qxx_init(void)
{
    if (sfud_init() == SFUD_SUCCESS)
    {
        sfud_w25qxx_self_inspection(0, sizeof(sfud_buf), sfud_buf);
    }
}
#endif

lfs_t lfs;
lfs_file_t file;
struct lfs_config cfg;

void user_lfs_init(void)
{
    if (sfud_init() == SFUD_SUCCESS)
    {
        extern int lfs_spi_flash_init(struct lfs_config * cfg);
        int err = lfs_spi_flash_init(&cfg);
        if (err)
        {
            log_i("lfs_spi_flash_init() failed"); 
        }

        // mount the filesystem
        err = lfs_mount(&lfs, &cfg);

        // reformat if we can't mount the filesystem
        // this should only happen on the first boot
        if (err)
        {
            lfs_format(&lfs, &cfg);
            lfs_mount(&lfs, &cfg);
        }

        // read current count
        uint32_t boot_count = 0;
        lfs_file_open(&lfs, &file, "boot_count", LFS_O_RDWR | LFS_O_CREAT);
        lfs_file_read(&lfs, &file, &boot_count, sizeof(boot_count));

        // update boot count
        boot_count += 1;
        lfs_file_rewind(&lfs, &file);
        lfs_file_write(&lfs, &file, &boot_count, sizeof(boot_count));

        // remember the storage is not updated until the file is closed successfully
        lfs_file_close(&lfs, &file);

        // release any resources we were using
        lfs_unmount(&lfs);

        // print the boot count
        log_i("boot_count: %d", boot_count);
    }
}

#if 1
// fal probe fdb
// fal read 0 64
// fal write 0 01 02 03 04 05 06 07
// fal erase 64
// fal bench 512 yes
#include "fal.h"
static void fal(uint8_t argc, char **argv)
{

#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
#define HEXDUMP_WIDTH 16
#define CMD_PROBE_INDEX 0
#define CMD_READ_INDEX 1
#define CMD_WRITE_INDEX 2
#define CMD_ERASE_INDEX 3
#define CMD_BENCH_INDEX 4

    int result = 0;
    static const struct fal_flash_dev *flash_dev = NULL;
    static const struct fal_partition *part_dev = NULL;
    size_t i = 0, j = 0;

    const char *help_info[] =
        {
            [CMD_PROBE_INDEX] = "fal probe [dev_name|part_name]   - probe flash device or partition by given name",
            [CMD_READ_INDEX] = "fal read addr size               - read 'size' bytes starting at 'addr'",
            [CMD_WRITE_INDEX] = "fal write addr data1 ... dataN   - write some bytes 'data' starting at 'addr'",
            [CMD_ERASE_INDEX] = "fal erase addr size              - erase 'size' bytes starting at 'addr'",
            [CMD_BENCH_INDEX] = "fal bench <blk_size>             - benchmark test with per block size",
        };

    if (fal_init_check() != 1)
    {
        printf("\n[Warning] FAL is not initialized or failed to initialize!\n\n");
        return;
    }

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

        if (!strcmp(operator, "probe"))
        {
            if (argc >= 3)
            {
                char *dev_name = argv[2];
                if ((flash_dev = fal_flash_device_find(dev_name)) != NULL)
                {
                    part_dev = NULL;
                }
                else if ((part_dev = fal_partition_find(dev_name)) != NULL)
                {
                    flash_dev = NULL;
                }
                else
                {
                    printf("Device %s NOT found. Probe failed.\n", dev_name);
                    flash_dev = NULL;
                    part_dev = NULL;
                }
            }

            if (flash_dev)
            {
                printf("Probed a flash device | %s | addr: %ld | len: %d |.\n", flash_dev->name,
                       flash_dev->addr, flash_dev->len);
            }
            else if (part_dev)
            {
                printf("Probed a flash partition | %s | flash_dev: %s | offset: %ld | len: %d |.\n",
                       part_dev->name, part_dev->flash_name, part_dev->offset, part_dev->len);
            }
            else
            {
                printf("No flash device or partition was probed.\n");
                printf("Usage: %s.\n", help_info[CMD_PROBE_INDEX]);
                fal_show_part_table();  
            }
        }
        else
        {
            if (!flash_dev && !part_dev)
            {
                printf("No flash device or partition was probed. Please run 'fal probe'.\n");
                return;
            }
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
                        if (flash_dev)
                        {
                            result = flash_dev->ops.read(addr, data, size);
                        }
                        else if (part_dev)
                        {
                            result = fal_partition_read(part_dev, addr, data, size);
                        }
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
                        if (flash_dev)
                        {
                            result = flash_dev->ops.write(addr, data, size);
                        }
                        else if (part_dev)
                        {
                            result = fal_partition_write(part_dev, addr, data, size);
                        }
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
                    if (flash_dev)
                    {
                        result = flash_dev->ops.erase(addr, size);
                    }
                    else if (part_dev)
                    {
                        result = fal_partition_erase(part_dev, addr, size);
                    }
                    if (result >= 0)
                    {
                        printf("Erase data success. Start from 0x%08X, size is %ld.\n", addr, size);
                    }
                }
            }
            else if (!strcmp(operator, "bench"))
            {
                if (argc < 3)
                {
                    printf("Usage: %s.\n", help_info[CMD_BENCH_INDEX]);
                    return;
                }
                else if ((argc > 3 && strcmp(argv[3], "yes")) || argc < 4)
                {
                    printf("DANGER: It will erase full chip or partition! Please run 'fal bench %d yes'.\n", strtol(argv[2], NULL, 0));
                    return;
                }
                /* full chip benchmark test */
                uint32_t start_time, time_cast;
                size_t write_size = strtol(argv[2], NULL, 0), read_size = strtol(argv[2], NULL, 0), cur_op_size;
                uint8_t *write_data = (uint8_t *)malloc(write_size), *read_data = (uint8_t *)malloc(read_size);

                if (write_data && read_data)
                {
                    for (i = 0; i < write_size; i++)
                    {
                        write_data[i] = i & 0xFF;
                    }
                    if (flash_dev)
                    {
                        size = flash_dev->len;
                    }
                    else if (part_dev)
                    {
                        size = part_dev->len;
                    }
                    /* benchmark testing */
                    printf("Erasing %ld bytes data, waiting...\n", size);
                    start_time = HAL_GetTick();
                    if (flash_dev)
                    {
                        result = flash_dev->ops.erase(0, size);
                    }
                    else if (part_dev)
                    {
                        result = fal_partition_erase(part_dev, 0, size);
                    }
                    if (result >= 0)
                    {
                        time_cast = HAL_GetTick() - start_time;
                        printf("Erase benchmark success, total time: %d.%03dS.\n", time_cast / 1000,
                               time_cast % 1000);
                    }
                    else
                    {
                        printf("Erase benchmark has an error. Error code: %d.\n", result);
                    }
                    /* write test */
                    printf("Writing %ld bytes data, waiting...\n", size);
                    start_time = HAL_GetTick();
                    for (i = 0; i < size; i += write_size)
                    {
                        if (i + write_size <= size)
                        {
                            cur_op_size = write_size;
                        }
                        else
                        {
                            cur_op_size = size - i;
                        }
                        if (flash_dev)
                        {
                            result = flash_dev->ops.write(i, write_data, cur_op_size);
                        }
                        else if (part_dev)
                        {
                            result = fal_partition_write(part_dev, i, write_data, cur_op_size);
                        }
                        if (result < 0)
                        {
                            break;
                        }
                    }
                    if (result >= 0)
                    {
                        time_cast = HAL_GetTick() - start_time;
                        printf("Write benchmark success, total time: %d.%03dS.\n", time_cast / 1000,
                               time_cast % 1000);
                    }
                    else
                    {
                        printf("Write benchmark has an error. Error code: %d.\n", result);
                    }
                    /* read test */
                    printf("Reading %ld bytes data, waiting...\n", size);
                    start_time = HAL_GetTick();
                    for (i = 0; i < size; i += read_size)
                    {
                        if (i + read_size <= size)
                        {
                            cur_op_size = read_size;
                        }
                        else
                        {
                            cur_op_size = size - i;
                        }
                        if (flash_dev)
                        {
                            result = flash_dev->ops.read(i, read_data, cur_op_size);
                        }
                        else if (part_dev)
                        {
                            result = fal_partition_read(part_dev, i, read_data, cur_op_size);
                        }
                        /* data check */
                        for (size_t index = 0; index < cur_op_size; index++)
                        {
                            if (write_data[index] != read_data[index])
                            {
                                printf("%d %d %02x %02x.\n", i, index, write_data[index], read_data[index]);
                            }
                        }

                        if (memcmp(write_data, read_data, cur_op_size))
                        {
                            result = -1;
                            printf("Data check ERROR! Please check you flash by other command.\n");
                        }
                        /* has an error */
                        if (result < 0)
                        {
                            break;
                        }
                    }
                    if (result >= 0)
                    {
                        time_cast = HAL_GetTick() - start_time;
                        printf("Read benchmark success, total time: %d.%03dS.\n", time_cast / 1000,
                               time_cast % 1000);
                    }
                    else
                    {
                        printf("Read benchmark has an error. Error code: %d.\n", result);
                    }
                }
                else
                {
                    printf("Low memory!\n");
                }
                free(write_data);
                free(read_data);
            }
            else
            {
                printf("Usage:\n");
                for (i = 0; i < sizeof(help_info) / sizeof(char *); i++)
                {
                    printf("%s\n", help_info[i]);
                }
                printf("\n");
                return;
            }
            if (result < 0)
            {
                printf("This operate has an error. Error code: %d.\n", result);
            }
        }
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), fal, fal, fal to probe read write erase bench);
#endif
