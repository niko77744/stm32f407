/**
 * @file shell_port.c
 * @author Letter (NevermindZZT@gmail.com)
 * @brief
 * @version 0.1
 * @date 2019-02-22
 *
 * @copyright (c) 2019 Letter
 *
 */

#define LOG_TAG "shell_port"
#include "FreeRTOS.h"
#include "task.h"
#include "shell.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include "log.h"
#include "elog.h"
#include "semphr.h"
#include "ring_buffer.h"

Shell shell;
char shellBuffer[512];

static SemaphoreHandle_t shellMutex;
static uint8_t rx_byte = 0;

RING_BUF_DECLARE(shellRxBuffer, 128);

/**
 * @brief 用户shell写
 *
 * @param data 数据
 * @param len 数据长度
 *
 * @return short 实际写入的数据长度
 */
short userShellWrite(char *data, unsigned short len)
{
    HAL_UART_Transmit(&huart6, (uint8_t *)data, len, 0x1FF);
    return len;
}

/**
 * @brief 用户shell读
 *
 * @param data 数据
 * @param len 数据长度
 *
 * @return short 实际读取到
 */
short userShellRead(char *data, unsigned short len)
{
    // if (HAL_UART_Receive(&huart6, (uint8_t *)data, len, 0x1FF) != HAL_OK)
    //     return 0;
    // else
    //     return 1;

    if (ring_buf_get(&shellRxBuffer, (uint8_t *)data, len) != len)
        return 0;
    else
        return 1;
}
/**
 * @brief 用户shell上锁
 *
 * @param shell shell
 *
 * @return int 0
 */
int userShellLock(Shell *shell)
{
    xSemaphoreTakeRecursive(shellMutex, portMAX_DELAY);
    return 0;
}

/**
 * @brief 用户shell解锁
 *
 * @param shell shell
 *
 * @return int 0
 */
int userShellUnlock(Shell *shell)
{
    xSemaphoreGiveRecursive(shellMutex);
    return 0;
}

void shell_recv_byte(void)
{
    ring_buf_put(&shellRxBuffer, &rx_byte, 1);
    HAL_UART_Receive_IT(&huart6, &rx_byte, 1);
}

/**
 * @brief 用户shell初始化
 *
 */
void userShellInit(void)
{
    shellMutex = xSemaphoreCreateMutex();

    shell.write = userShellWrite;
    shell.read = userShellRead;
    shell.lock = userShellLock;
    shell.unlock = userShellUnlock;
    HAL_UART_Receive_IT(&huart6, &rx_byte, 1);
    shellInit(&shell, shellBuffer, 512);
}
