#define LOG_TAG "iap"
#include <limits.h>
#include "iap.h"
#include "elog.h"
#include "fatfs.h"
#include "ring_buffer.h"
#include "shell.h"
#include "shell_port.h"
#include "fal_cfg.h"
#include "fal.h" // flash抽象层 统一flash接口

typedef struct
{
    struct ring_buf *const buf; // 指针本身是常量，不能指向其他地址
    uint32_t capacity;
    uint32_t used;
    uint32_t space;
} iap_ringbuffer_t;
typedef void (*iapfun)(void); /* 定义一个函数类型的参数 */

struct bspatch_stream
{
    void *opaque_r;
    int (*read)(const struct bspatch_stream *stream, void *buffer, int length);
};
static int64_t offtin(uint8_t *buf);
static int bspatch(uint32_t old_addr, int64_t oldsize, uint32_t new_addr, int64_t newsize, struct bspatch_stream *stream);

/**
 * @brief 差分包读取上下文结构
 *
 * 用于记录差分包数据的读取位置和缓冲区指针，
 * 供bspatch_stream.read回调函数使用
 */
typedef struct
{
    size_t read_offset;  /**< 当前读取偏移量 */
    uint32_t patch_addr; /**< 差分数据缓冲区指针 */
} patch_reader_context;

RING_BUF_DECLARE(iap_ring_buffer, (IAP_RX_LEN * 4));               // _ring_buffer_data_iap_ring_buffer 串口接收环形缓冲区
static uint8_t iap_write_buffer[IAP_RX_LEN];                       // 从ring_buf中读取写入flash的缓冲区
uint8_t iap_dma_rx_buffer[IAP_RX_LEN] __attribute__((aligned(4))); // 禁止编译器优化，对齐4字节; 串口dma接收缓冲区
static uint32_t iap_dma_rx_size = 0;                               // 串口接收到数据的大小
static uint32_t app_write_flash_addr = 0;                          // 写入app的falsh的地址 相对偏移一个boot
static iap_ringbuffer_t iap_rb = {.buf = &iap_ring_buffer};
static iapfun jump2app = NULL;
static patch_reader_context reader_context = {0};
struct bspatch_stream bspatch_stream_obj = {0};

#define NEW_START_ADDR (0x040000UL)
/**
 * @brief       跳转到应用程序段(执行APP)
 * @param       appxaddr : 应用程序的起始地址
 * @retval      无
 */
static void iap_load_app(uint32_t appxaddr)
{
    // 判断是否为0x08XXXXXX.
    if (((*(__IO uint32_t *)(appxaddr + 4)) & 0xff000000) == 0x08000000)
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
}

void JumpToApp(void)
{
    iap_load_app(FAL_APP1_START_ADDR);
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), jumpToApp, JumpToApp, JumpToApp);

/**
 * @brief       IAP写入APP BIN
 * @param       appxaddr : 应用程序的起始地址
 * @param       appbuf   : 应用程序CODE
 * @param       appsize  : 应用程序大小(字节)
 * @retval      无
 */
static void iap_write_appbin(uint32_t appxaddr, uint8_t *appbuf, uint32_t appsize)
{
    if (fal_init_check() != 1)
        return;

    static const struct fal_partition *part_dev = NULL;
    part_dev = fal_partition_find("download");

    if (part_dev != NULL)
    {
        if (fal_partition_write(part_dev, appxaddr, appbuf, appsize) < 0)
        {
            log_e("write flash %02x error", appxaddr);
        }
    }
}

void iap_rx_event_callback(uint32_t Size)
{
    iap_dma_rx_size += Size;
    ring_buf_put(iap_rb.buf, iap_dma_rx_buffer, Size);
}

static int read_patch(const struct bspatch_stream *stream, void *buffer, int length)
{
    static const struct fal_partition *part_dowmload = NULL;
    part_dowmload = fal_partition_find("download");
    patch_reader_context *ctx = (patch_reader_context *)stream->opaque_r;
    // memcpy(buffer, ctx->patch_buffer + ctx->read_offset, length);
    fal_partition_read(part_dowmload, ctx->patch_addr + ctx->read_offset, buffer, length);

    ctx->read_offset += length;
    return 0;
}

void iap_init(void)
{
    extern DMA_HandleTypeDef hdma_usart1_rx;
    ring_buf_reset(iap_rb.buf);
    memset(iap_dma_rx_buffer, 0, IAP_RX_LEN);
    memset(iap_write_buffer, 0, IAP_RX_LEN);
    app_write_flash_addr = 0;
    iap_dma_rx_size = 0;
    __HAL_DMA_ENABLE_IT(&hdma_usart1_rx, DMA_IT_TE); // 开启传输错误中断(必加，容错)

    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, iap_dma_rx_buffer, sizeof(iap_dma_rx_buffer));
    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
}

void iap_uart_proceess(void)
{
    iap_rb.space = ring_buf_space_get(iap_rb.buf);
    iap_rb.capacity = ring_buf_capacity_get(iap_rb.buf);
    iap_rb.used = ring_buf_size_get(iap_rb.buf);
    uint16_t len = ring_buf_get(iap_rb.buf, iap_write_buffer, IAP_RX_LEN);
    if (len > 0)
    {
        iap_write_appbin(app_write_flash_addr, iap_write_buffer, len);
        memset(iap_write_buffer, 0xff, IAP_RX_LEN);
        app_write_flash_addr += len;
    }
}

/**
 * @brief 解压并还原文件，用户使用差分升级时唯一需要关心的接口 const uint8_t *old, uint32_t oldsize, const uint8_t *patch, uint32_t patchsize, uint32_t newaddr
 *
 * @param old 设备中执行区代码所在的地址，用户可指定flash执行区的地址，方便算法读出来当前
 * @param oldsize 设备中执行区代码的长度
 * @param patch 设备中已经下载的差分包所在的flash地址
 * @param patchsize 设备中已经下载的差分包的长度 可在差分包bin头获取
 * @param newaddr 还原后的新固件写入的地址
 * @return int 还原的文件大小
 */
// 这里把参数修改 因为目前使用的stm32f4最小擦除单位是128K,无法做到一边在旧固件上读取解析写入(如果是2k的最小擦除,就可以),只能借助外部flash将差分包存放在外部flash当中,解析完数据(可以在外部或者内部flash(app2)当中,我这里选择将解析完的数据也放在外部再搬回app1)
int ota_patch(uint8_t argc, char **argv)
{
    if (argc != 2)
    {
        shellPrint(&shell, "Use example otapatch 0x1000(oldsize)\n");
        return 0;
    }
    else
    {
        uint32_t oldsize = 0;
        oldsize = strtol(argv[1], NULL, 0); // 这里是新固件的大小

        // 存放patch差分包和new新固件 在外部flash
        static const struct fal_partition *part_dowmload = NULL;
        part_dowmload = fal_partition_find("download");

        uint8_t header[24];
        int32_t newsize = 0; // 新固件的大小

        fal_partition_read(part_dowmload, 0, header, sizeof(header));

        if (memcmp(header, "ENDSLEY/BSDIFF43", 16) != 0)
        {
            shellPrint(&shell, "ENDSLEY/BSDIFF43 err: %s\n", header);
            return 0;
        }

        // 计算新固件长度
        newsize = offtin(header + 16);
        shellPrint(&shell, "new firmware size: dec=%lu, hex=0x%08x\n", (unsigned long)newsize, newsize);
        if (newsize < 0)
        {
            shellPrint(&shell, "newsize err\n");
            return 0;
        }

        // 初始化差分数据读取上下文
        reader_context.read_offset = 0;
        reader_context.patch_addr = 0 + sizeof(header);
        bspatch_stream_obj.opaque_r = &reader_context;
        bspatch_stream_obj.read = read_patch;

        // new的缓冲区填NULL,因为在单片机上没有足够的RAM来直接生成新固件 所以需要修改源码直接将new写入flash
        if (bspatch(0, oldsize, 0, newsize, &bspatch_stream_obj) != 0)
        {
            shellPrint(&shell, "bspatch error\n");
        }
        shellPrint(&shell, "bspatch success\n");
        return newsize;
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), otapatch, ota_patch, ota_patch);

static int64_t offtin(uint8_t *buf)
{
    int64_t y;

    y = buf[7] & 0x7F;
    y = y * 256;
    y += buf[6];
    y = y * 256;
    y += buf[5];
    y = y * 256;
    y += buf[4];
    y = y * 256;
    y += buf[3];
    y = y * 256;
    y += buf[2];
    y = y * 256;
    y += buf[1];
    y = y * 256;
    y += buf[0];

    if (buf[7] & 0x80)
        y = -y;

    return y;
}

static uint8_t bspatch_buffer[IAP_RX_LEN] = {0};
static int bspatch(uint32_t old_addr, int64_t oldsize, uint32_t new_addr, int64_t newsize, struct bspatch_stream *stream)
{
    const struct fal_partition *part_app1 = NULL;
    const struct fal_partition *part_new = NULL;
    part_app1 = fal_partition_find("app1");
    part_new = fal_partition_find("new");
    uint8_t buf[8];
    int64_t oldpos, newpos;
    int64_t ctrl[3];
    int64_t i;
    uint32_t current_flash_addr = new_addr;

    oldpos = 0;
    newpos = 0;
    while (newpos < newsize)
    {
        // 读取控制数据
        for (i = 0; i <= 2; i++)
        {
            if (stream->read(stream, buf, 8))
                return -1;
            ctrl[i] = offtin(buf);
        };

        /* Sanity-check */
        if (ctrl[0] < 0 || ctrl[0] > INT_MAX ||
            ctrl[1] < 0 || ctrl[1] > INT_MAX ||
            newpos + ctrl[0] > newsize)
            return -1;

        // 处理差异数据
        int64_t diff_len = ctrl[0];
        while (diff_len > 0)
        {
            // 计算本次处理的长度
            int32_t chunk_size = (diff_len > IAP_RX_LEN) ? IAP_RX_LEN : diff_len;

            // 读取差异数据到缓冲区
            if (stream->read(stream, bspatch_buffer, chunk_size))
                return -1;

            // 与旧数据相加
            for (i = 0; i < chunk_size; i++)
            {
                if (oldpos + i >= 0 && oldpos + i < oldsize)
                {
                    uint8_t read_byte = 0;
                    fal_partition_read(part_app1, (old_addr + oldpos + i), &read_byte, 1);
                    bspatch_buffer[i] += read_byte;
                }
            }

            // 写入Flash
            if (fal_partition_write(part_new, current_flash_addr, bspatch_buffer, chunk_size) < 0)
                return -1;

            diff_len -= chunk_size;
            current_flash_addr += chunk_size;
            oldpos += chunk_size;
            newpos += chunk_size;
        }

        /* Sanity-check */
        if (newpos + ctrl[1] > newsize)
            return -1;

        // 处理额外数据
        int64_t extra_len = ctrl[1];
        while (extra_len > 0)
        {
            // 计算本次处理的长度
            int32_t chunk_size = (extra_len > IAP_RX_LEN) ? IAP_RX_LEN : extra_len;

            // 读取额外数据直接写入Flash
            if (stream->read(stream, bspatch_buffer, chunk_size))
                return -1;

            // 写入Flash
            if (fal_partition_write(part_new, current_flash_addr, bspatch_buffer, chunk_size) < 0)
                return -1;

            // 更新位置
            current_flash_addr += chunk_size;
            extra_len -= chunk_size;
            newpos += chunk_size;
        }

        /* Adjust pointers */
        oldpos += ctrl[2];
    };

    return 0;
}

// appmove download 2000
void app_move(uint8_t argc, char **argv)
{
#define BUFFER_SZIE (128)
    if (fal_init_check() != 1)
    {
        shellPrint(&shell, "\n[Warning] FAL is not initialized or failed to initialize!\n\n");
        return;
    }

    if (argc != 3)
    {
        shellPrint(&shell, "Use example appmove download 0x1000\n");
    }
    else
    {
        const char *part_name = argv[1];
        static const struct fal_partition *part_dev = NULL;
        static const struct fal_partition *part_app1 = NULL;
        part_dev = fal_partition_find(part_name);
        part_app1 = fal_partition_find("app1");
        if (part_dev == NULL || part_app1 == NULL)
        {
            shellPrint(&shell, "No flash device or partition was probed.\n");
        }
        else
        {
            uint32_t app_size = strtol(argv[2], NULL, 0); // 这里是新固件的大小
            uint8_t *buffer = pvPortMalloc(BUFFER_SZIE);
            if (buffer)
            {
// 初始化：先输出空的进度条框架，占据一行（仅执行一次）
#define PROGRESS_BAR_LEN 20
// 固定整行输出的前缀（方便计算总长度）
#define PROGRESS_PREFIX "Upgrade: ["
#define PROGRESS_SUFFIX "] 100%%"
                uint8_t progress_bar[PROGRESS_BAR_LEN + 1] = {0};
                uint32_t progress = 0;
                uint32_t last_progress = 0; // 记录上一次更新的进度，避免频繁刷新
                // 1. 初始化：先输出全空的进度条，并用空格填满整行（固定长度）
                // 计算整行总长度：前缀(9) + 进度条(20) + 后缀(6) + 预留空格(5) = 40字符，足够覆盖所有情况
                memset(progress_bar, '-', PROGRESS_BAR_LEN); // 初始化为全"-"
                progress_bar[PROGRESS_BAR_LEN] = '\0';
                // 输出初始化行，\r回到行首，后续所有更新都基于这一行
                shellPrint(&shell, "\r%s%s]   0%%%*s", PROGRESS_PREFIX, progress_bar, 5, "");

                fal_partition_erase_all(part_app1);
                for (uint32_t i = 0; i < app_size; i += BUFFER_SZIE)
                {
                    // 计算本次要操作的实际字节数
                    uint32_t remaining = app_size - i;
                    uint32_t chunk_size = (remaining < BUFFER_SZIE) ? remaining : BUFFER_SZIE;

                    fal_partition_read(part_dev, i, buffer, chunk_size);
                    fal_partition_write(part_app1, i, buffer, chunk_size);

                    // 计算进度（严格保证不超100%）
                    if (app_size > 0)
                    {
                        progress = ((i + chunk_size) * 100) / app_size;
                        progress = progress > 100 ? 100 : progress;
                    }

                    // 仅当进度变化≥1%时才更新，避免频繁输出导致乱码
                    if (progress != last_progress)
                    {
                        last_progress = progress; // 更新上次进度

                        // 生成进度条字符
                        uint32_t filled = progress * PROGRESS_BAR_LEN / 100;
                        memset(progress_bar, '-', PROGRESS_BAR_LEN); // 先填充未完成符
                        memset(progress_bar, '#', filled);           // 填充已完成符
                        progress_bar[PROGRESS_BAR_LEN] = '\0';       // 字符串结束符

                        // 4. 固定长度输出：用%*s填充空格，确保每次输出的字符数完全一致
                        // \r回到行首 + 固定前缀 + 进度条 + 百分比（3位） + 5个空格覆盖残留字符
                        shellPrint(&shell, "\r%s%s] %3d%%%*s",
                                   PROGRESS_PREFIX, progress_bar, progress, 5, "");
                    }
                }

                // 5. 最终完成：强制输出100%，并换行（确保格式整洁）
                memset(progress_bar, '#', PROGRESS_BAR_LEN); // 全满进度条
                shellPrint(&shell, "\r%s%s] 100%%%*s\n", PROGRESS_PREFIX, progress_bar, 5, "");

                vPortFree(buffer);
                fal_partition_erase(part_dev, 0, app_size);
                shellPrint(&shell, "appmove success\n");
            }
        }
    }
}
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), appmove, app_move, app_move);

uint16_t crc16_ccitt(const uint8_t *data, size_t length)
{
    uint16_t crc = 0;
    for (size_t i = 0; i < length; i++)
    {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
            {
                crc = (crc << 1) ^ CRC16_CCITT;
            }
            else
            {
                crc <<= 1;
            }
        }
    }
    return crc;
}

uint32_t crc32_ieee(const uint8_t *data, size_t length)
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++)
    {
        crc ^= data[i];
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 1)
            {
                crc = (crc >> 1) ^ CRC32_IEEE;
            }
            else
            {
                crc = crc >> 1;
            }
        }
    }
    return crc ^ 0xFFFFFFFF;
}
