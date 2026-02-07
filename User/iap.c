#define LOG_TAG "iap"
#include "iap.h"
#include "elog.h"
#include "fatfs.h"
#include <stm32f407xx.h>
#include "ring_buffer.h"
#include "fal_cfg.h"
#include "shell.h"
#include "fal.h"

#pragma diag_suppress 177 // 抑制本文件中的 177 警告 -- 忽略未使用的函数警告

RING_BUF_DECLARE(iap_ring_buffer, (IAP_RX_LEN * 4));            // _ring_buffer_data_iap_ring_buffer 串口接收环形缓冲区
static uint8_t iap_write_buffer[IAP_RX_LEN];                    // 从ring_buf中读取写入flash的缓冲区
uint8_t iap_dma_rx_buf[IAP_RX_LEN] __attribute__((aligned(4))); // 禁止编译器优化，对齐4字节; 串口dma接收缓冲区

typedef void (*iapfun)(void); /* 定义一个函数类型的参数 */
iapfun jump2app = NULL;
/**
 * @brief       跳转到应用程序段(执行APP)
 * @param       appxaddr : 应用程序的起始地址
 * @retval      无
 */
void iap_load_app(uint32_t appxaddr)
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
SHELL_EXPORT_CMD(SHELL_CMD_PERMISSION(0) | SHELL_CMD_TYPE(SHELL_TYPE_CMD_MAIN), JumpToApp, JumpToApp, JumpToApp);

// static uint32_t g_iapbuf[IAP_RX_LEN] = {0}; /* 2K字节缓存 */
/**
 * @brief       IAP写入APP BIN
 * @param       appxaddr : 应用程序的起始地址
 * @param       appbuf   : 应用程序CODE
 * @param       appsize  : 应用程序大小(字节)
 * @retval      无
 */
void iap_write_appbin(uint32_t appxaddr, uint8_t *appbuf, uint32_t appsize)
{
    // static const struct fal_flash_dev *flash_dev = NULL;
    static const struct fal_partition *part_dev = NULL;
    if (fal_init_check() != 1)
        return;

    part_dev = fal_partition_find("app1");
    if (part_dev != NULL)
    {
        if (fal_partition_write(part_dev, appxaddr, appbuf, appsize) < 0)
        {
            while (1)
                ;
        }
    }
}

uint32_t iap_rx_size = 0;
void iap_rx_event_callback(uint32_t Size)
{
    iap_rx_size += Size;
    ring_buf_put(&iap_ring_buffer, iap_dma_rx_buf, Size);
}

void iap_init(void)
{
    extern DMA_HandleTypeDef hdma_usart1_rx;
    ring_buf_reset(&iap_ring_buffer);
    memset(iap_dma_rx_buf, 0, IAP_RX_LEN);
    __HAL_DMA_ENABLE_IT(&hdma_usart1_rx, DMA_IT_TE); // 开启传输错误中断(必加，容错)

    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, iap_dma_rx_buf, sizeof(iap_dma_rx_buf));
    __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
}

uint32_t record_size = 0;
uint32_t space_size = 0;
void iap_uart_proceess(void)
{
    space_size = ring_buf_space_get(&iap_ring_buffer);
    uint16_t len = ring_buf_get(&iap_ring_buffer, iap_write_buffer, IAP_RX_LEN);
    if (len > 0)
    {
        iap_write_appbin(record_size, iap_write_buffer, len);
        memset(iap_write_buffer, 0xff, IAP_RX_LEN);
        record_size += len;
    }
}

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
