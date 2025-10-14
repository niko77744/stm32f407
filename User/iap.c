#define LOG_TAG "iap"
#include "iap.h"
#include "elog.h"
#include "fatfs.h"
#include "flashdb.h"

#pragma diag_suppress 177 // 抑制本文件中的 177 警告 -- 忽略未使用的函数警告

#define IAP_BUF_SIZE 512 // 512
#define BUFFER_COUNT 4   // 增加缓冲区数量
#define BUFFER_SIZE 256  // 每个缓冲区大小
typedef struct
{
    uint8_t data[BUFFER_SIZE];
    uint16_t size;
    uint8_t used;
} log_buffer_t;

iap_t iap_handle;
__align(4) uint32_t g_iapbuf[IAP_BUF_SIZE] = {0}; /* 2K字节缓存 */
__align(4) uint8_t g_logbuf[BUFFER_SIZE] = {0};
log_buffer_t g_log_buffers[BUFFER_COUNT] = {0};
uint8_t g_current_write_index = 0;
uint8_t g_current_read_index = 0;
uint8_t g_buffer_available = BUFFER_COUNT;
__IO uint8_t buffer_full_count = 0; // 信号量或标志位，用于缓冲区同步

typedef void (*iapfun)(void); /* 定义一个函数类型的参数 */
iapfun jump2app = NULL;

uint32_t write_app_addr = 0;
uint32_t firmware_size = 0;
uint8_t num = 0;
uint16_t num_size[16] = {0};

void iap_init(iap_source_t source)
{
    iap_handle.source = source;
    iap_handle.err_code = iap_err_none;
    iap_handle.state = iap_state_idle;
    write_app_addr = FLASH_APP1_ADDR;
    if (source == iap_from_uart)
    {
        __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart6, g_logbuf, sizeof(g_logbuf));
        __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
    }
}

void log_rx_event_callback(uint16_t Size)
{
    num_size[num++] = Size;
    // 检查是否有可用缓冲区
    if (g_buffer_available == 0)
    {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart6, g_logbuf, sizeof(g_logbuf));
        __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
        return;
    }

    // 复制数据到当前写缓冲区
    memcpy(g_log_buffers[g_current_write_index].data, g_logbuf, Size);
    g_log_buffers[g_current_write_index].size = Size;
    g_log_buffers[g_current_write_index].used = 1;

    // 更新写索引
    g_current_write_index = (g_current_write_index + 1) % BUFFER_COUNT;

    // 更新可用缓冲区计数
    if (g_buffer_available > 0)
    {
        g_buffer_available--;
    }

    buffer_full_count++;

    // 立即重新启动DMA接收
    memset(g_logbuf, 0, sizeof(g_logbuf));
    HAL_UARTEx_ReceiveToIdle_DMA(&huart6, g_logbuf, sizeof(g_logbuf));
    __HAL_DMA_DISABLE_IT(huart6.hdmarx, DMA_IT_HT);
}

void iap_uart_proceess(void)
{
    while (buffer_full_count > 0 && g_log_buffers[g_current_read_index].used)
    {
        // 写入Flash
        iap_write_appbin(write_app_addr,
                         g_log_buffers[g_current_read_index].data,
                         g_log_buffers[g_current_read_index].size);

        // 更新地址和大小
        write_app_addr += g_log_buffers[g_current_read_index].size;
        firmware_size += g_log_buffers[g_current_read_index].size;

        // 标记缓冲区为空
        g_log_buffers[g_current_read_index].used = 0;
        g_log_buffers[g_current_read_index].size = 0;

        // 更新读索引
        g_current_read_index = (g_current_read_index + 1) % BUFFER_COUNT;

        // 更新缓冲区计数
        if (g_buffer_available < BUFFER_COUNT)
        {
            g_buffer_available++;
        }

        buffer_full_count--;
    }
}

/**
 * @brief       验证Flash中写入的数据与文件是否一致
 * @param       filename: 文件名
 * @param       flash_addr: Flash起始地址
 * @param       total_bytes: 总字节数
 * @retval      true: 验证成功, false: 验证失败
 */
uint8_t iap_verify_integrity(const char *filename, uint32_t flash_addr, uint32_t total_bytes)
{
    FRESULT res;
    FIL file;
    UINT bytes_read;
    uint32_t file_offset = 0;
    uint32_t verify_bytes = 0;
    uint8_t verify_result = 1;

    // 打开文件进行验证
    res = f_open(&file, filename, FA_READ);
    if (res != FR_OK)
    {
        log_e("Error opening file for verification: %d", res);
        return false;
    }

    log_i("Starting integrity verification...");

    // 计算需要读取的字数（32位对齐）
    uint32_t words_to_read = (total_bytes + 3) / 4; // 向上取整
    uint32_t read_buffer[256];                      // 读取缓冲区（1024字节）
    uint8_t file_buffer[1024];                      // 文件读取缓冲区

    while (file_offset < total_bytes)
    {
        uint32_t bytes_remaining = total_bytes - file_offset;
        uint32_t chunk_size = (bytes_remaining > sizeof(file_buffer)) ? sizeof(file_buffer) : bytes_remaining;

        // 从文件读取数据
        res = f_read(&file, file_buffer, chunk_size, &bytes_read);
        if (res != FR_OK || bytes_read != chunk_size)
        {
            log_e("File read error during verification: %d, read %u, expected %lu",
                  res, bytes_read, chunk_size);
            verify_result = 0;
            break;
        }

        // 计算当前块需要读取的字数
        uint32_t current_words = (chunk_size + 3) / 4;
        if (current_words > sizeof(read_buffer) / sizeof(read_buffer[0]))
        {
            current_words = sizeof(read_buffer) / sizeof(read_buffer[0]);
            chunk_size = current_words * 4;
        }

        // 从Flash读取数据
        stmflash_read(flash_addr, read_buffer, current_words);

        // 比较数据
        if (memcmp(file_buffer, read_buffer, chunk_size) != 0)
        {
            log_e("Data verification failed at offset 0x%08lX", file_offset);

            verify_result = false;
            break;
        }

        file_offset += chunk_size;
        flash_addr += chunk_size;
        verify_bytes += chunk_size;

        // 显示验证进度
        if (verify_bytes % 1024 == 0) // 每1KB显示一次进度
        {
            log_i("Verification progress: %lu/%lu bytes (%.1f%%)",
                  verify_bytes, total_bytes,
                  (float)verify_bytes * 100 / total_bytes);
        }
    }

    f_close(&file);

    if (verify_result)
    {
        log_i("Integrity verification: SUCCESS - %lu bytes verified", verify_bytes);
    }
    else
    {
        log_e("Integrity verification: FAILED at %lu/%lu bytes", verify_bytes, total_bytes);
    }

    return verify_result;
}

void iap_process(void)
{
    if (iap_handle.source == iap_from_sdcard)
    {
        FRESULT res;
        FIL file;
        UINT byteswritten, bytesread;
        FILINFO fno;
        DIR dir;
        UINT bytes_read;
        uint32_t total_bytes = 0;
        uint32_t write_addr = FLASH_APP1_ADDR;
        bool verify_success = true;

        // 1. 检查文件是否存在
        res = f_stat("app.bin", &fno);
        if (res != FR_OK)
        {
            log_e("app.bin not found: %d", res);
            return;
        }

        // 2. 打开文件
        res = f_open(&file, "app.bin", FA_READ);
        if (res != FR_OK)
        {
            log_e("Error opening file: %d", res);
            return;
        }
        log_i("Open app.bin succeed, file size: %lu bytes", fno.fsize);

        // 3. 擦除目标Flash区域（如果需要）
        // iap_erase_app_region(FLASH_APP1_ADDR, fno.fsize);

        // 4. 读取文件并写入Flash
        while (1)
        {
            // 从文件读取数据到g_logbuf
            res = f_read(&file, g_logbuf, sizeof(g_logbuf), &bytes_read);
            if (res != FR_OK || bytes_read == 0)
            {
                if (res != FR_OK)
                {
                    log_e("File read error: %d", res);
                }
                break;
            }

            // 将数据写入Flash
            iap_write_appbin(write_addr, g_logbuf, bytes_read);

            total_bytes += bytes_read;
            write_addr += bytes_read;

            // 显示进度
            if (total_bytes % 1024 == 0)
            { // 每1KB显示一次进度
                log_i("Writing progress: %lu/%lu bytes (%.1f%%)",
                      total_bytes, fno.fsize,
                      (float)total_bytes * 100 / fno.fsize);
            }

            // 清空缓冲区以备下次使用
            memset(g_logbuf, 0, sizeof(g_logbuf));
            memset(g_iapbuf, 0, sizeof(g_iapbuf));
        }

        // 5. 完成处理
        log_i("Firmware update completed: %lu bytes written", total_bytes);

        // 6. 验证文件完整性
        if (total_bytes != fno.fsize)
        {
            log_w("File size mismatch: expected %lu, wrote %lu", fno.fsize, total_bytes);
            verify_success = false;
        }
        else
        {
            log_i("File size verification: OK");

            // 执行数据完整性验证
            verify_success = iap_verify_integrity("app.bin", FLASH_APP1_ADDR, total_bytes);
        }

        // 7. 关闭文件和清理
        f_close(&file);

        // 8. 根据验证结果处理
        if (verify_success)
        {
            log_i("Firmware update and verification: SUCCESS");
            // 可以设置成功标志或直接跳转
            // iap_set_update_flag(true);
            // iap_jump_to_app(FLASH_APP1_ADDR);
        }
        else
        {
            log_e("Firmware verification: FAILED");
            // 可以设置失败标志或保持原有固件
            // iap_set_update_flag(false);
        }
    }
}

iap_err_e iap_recv_program_flash(uint8_t *data, uint32_t size)
{
    return iap_err_none;
}

/*
*********************************************************************************************************
*	函 数 名: JumpToBootloader
*	功能说明: 跳转到系统BootLoader
*	形    参: 无
*	返 回 值: 无
*********************************************************************************************************
*/

static void JumpToSysBootloader(void)
{
    uint32_t i = 0;
    void (*SysMemBootJump)(void);        /* 声明一个函数指针 */
    __IO uint32_t BootAddr = 0x1FFF0000; /* STM32F4的系统BootLoader地址 */

    /* 关闭全局中断 __disable_irq(); */
    __set_PRIMASK(1);

    /* 关闭滴答定时器，复位到默认值 */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    /* 设置所有时钟到默认状态，使用HSI时钟 */
    HAL_RCC_DeInit();

    /* 关闭所有中断，清除所有中断挂起标志 */
    for (i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    /* 使能全局中断 __enable_irq */
    __set_PRIMASK(0);

    /* 跳转到系统BootLoader，首地址是MSP，地址+4是复位中断服务程序地址 */
    SysMemBootJump = (void (*)(void))(*((uint32_t *)(BootAddr + 4)));

    /* 设置主堆栈指针 */
    __set_MSP(*(uint32_t *)BootAddr);

    /* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
#if SUPPORT_OS == 1
    __set_CONTROL(0);
#endif

    /* 跳转到系统BootLoader */
    SysMemBootJump();

    /* 跳转成功的话，不会执行到这里，用户可以在这里添加代码 */
    while (1)
        ;
}

/**
 * @brief       跳转到应用程序段(执行APP)
 * @param       appxaddr : 应用程序的起始地址
 * @retval      无
 */
void iap_load_app(uint32_t appxaddr)
{
    if (((*(volatile uint32_t *)appxaddr) & 0x2FFE0000) == 0x20000000) /* 检查栈顶地址是否合法.可以放在内部SRAM共64KB(0x20000000) */
    {
        /* 关闭全局中断 __disable_irq(); */
        __set_PRIMASK(1);

        /* 关闭滴答定时器，复位到默认值 */
        SysTick->CTRL = 0;
        SysTick->LOAD = 0;
        SysTick->VAL = 0;

        /* 设置所有时钟到默认状态，使用HSI时钟 */
        HAL_RCC_DeInit();

        /* 关闭所有中断，清除所有中断挂起标志 */
        for (uint8_t i = 0; i < 8; i++)
        {
            NVIC->ICER[i] = 0xFFFFFFFF;
            NVIC->ICPR[i] = 0xFFFFFFFF;
        }

        /* 使能全局中断 __enable_irq */
        __set_PRIMASK(0);

        /* 用户代码区第二个字为程序开始地址(复位地址) */
        jump2app = (iapfun) * (volatile uint32_t *)(appxaddr + 4);

        /* 初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址) */
        __set_MSP(*(volatile uint32_t *)appxaddr);

        /* 在RTOS工程，这条语句很重要，设置为特权级模式，使用MSP指针 */
#if SUPPORT_OS == 1
        __set_CONTROL(0);
#endif

        /* 跳转到APP */
        jump2app();
        while (1)
            ;
    }
}

/**
 * @brief       IAP写入APP BIN
 * @param       appxaddr : 应用程序的起始地址
 * @param       appbuf   : 应用程序CODE
 * @param       appsize  : 应用程序大小(字节)
 * @retval      无
 */
void iap_write_appbin(uint32_t appxaddr, uint8_t *appbuf, uint32_t appsize)
{
    uint32_t t;
    uint16_t i = 0;
    uint32_t temp;
    uint32_t fwaddr = appxaddr; /* 当前写入的地址 */
    uint8_t *dfu = appbuf;

    for (t = 0; t < appsize; t += 4)
    {
        temp = (uint32_t)dfu[3] << 24;
        temp |= (uint32_t)dfu[2] << 16;
        temp |= (uint32_t)dfu[1] << 8;
        temp |= (uint32_t)dfu[0];
        dfu += 4; /* 偏移2个字节 */
        g_iapbuf[i++] = temp;

        if (i == 512)
        {
            i = 0;
            stmflash_write(fwaddr, g_iapbuf, 512);
            fwaddr += 2048; /* 偏移2048  16 = 2 * 8  所以要乘以2 */
        }
    }

    if (i)
    {
        stmflash_write(fwaddr, g_iapbuf, i); /* 将最后的一些内容字节写进去 */
    }
}
