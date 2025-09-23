#define LOG_TAG "BLE"
#include "ble.h"
#include "elog.h"
#include "lwevt.h"
#pragma diag_suppress 550 // 抑制本文件中的 550 警告 -- 忽略未使用的函数警告

#define BLE_BUFFER_SIZE 128
uint8_t ble_rx_buffer[BLE_BUFFER_SIZE] = {0};
uint8_t ble_tx_buffer[BLE_BUFFER_SIZE] = {0};
uint8_t ble_rx_len = 0;
uint8_t ble_tx_len = 0;
MultiTimer ble_timer;
static lwevt_t ble_evt_local;

GPIO_PinState get_ble_device_state(void)
{
    return HAL_GPIO_ReadPin(BLE_Connect_GPIO_Port, BLE_Connect_Pin);
}

void ble_connect_callback(void)
{
    log_i("BLE connected");
    beep_start(beep_long);
}

void ble_disconnect_callback(void)
{
    log_i("BLE disconnected");
    beep_start(beep_short);
}

static void ble_ticks_callback(MultiTimer *timer, void *userData)
{
    static uint8_t prev_state = 0;
    uint8_t curr_state = get_ble_device_state();

    if (curr_state != prev_state)
    {
        if (curr_state == GPIO_PIN_SET)
            lwevt_dispatch_ex(&ble_evt_local, LWEVT_TYPE_BLE_CONNECT_BASIC); // 用本地句柄分发事件
        else
            lwevt_dispatch_ex(&ble_evt_local, LWEVT_TYPE_BLE_DISCONNECT_BASIC); // 用本地句柄分发事件
        prev_state = curr_state;
    }
    multiTimerStart(&ble_timer, 10, ble_ticks_callback, NULL);
}

// 不定长数据接收完成回调函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART3)
    {
        ble_rx_len = Size;
        ble_evt_local.msg.ble_rx.data = ble_rx_buffer;
        ble_evt_local.msg.ble_rx.data_len = ble_rx_len;
        lwevt_dispatch_ex(&ble_evt_local, LWEVT_TYPE_EXT_BLE_RX); // 用本地句柄分发事件
    }
    else if (huart->Instance == UART4)
    {
        log_i("ESP8266 data received %s", esp8266_buf);
        memset(esp8266_buf, 0, sizeof(esp8266_buf));
        // 重新启动接收，使用Ex函数，接收不定长数据
        // HAL_UARTEx_ReceiveToIdle_IT(&huart4, esp8266_buf, sizeof(esp8266_buf));
        HAL_UARTEx_ReceiveToIdle_DMA(&huart4, esp8266_buf, sizeof(esp8266_buf));
    }
    else if (huart->Instance == USART6)
    {
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        log_i("BLE data send over");
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4)
    {
        __HAL_UART_CLEAR_FEFLAG(huart);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart4, esp8266_buf, sizeof(esp8266_buf));
    }
}

static void ble_evt_callback(lwevt_t *e)
{
    switch ((unsigned)e->type)
    {
    case LWEVT_TYPE_BLE_CONNECT_BASIC:
    {
        ble_connect_callback();
        break;
    }
    case LWEVT_TYPE_BLE_DISCONNECT_BASIC:
    {
        ble_disconnect_callback();
        break;
    }
    case LWEVT_TYPE_EXT_BLE_RX:
    {
        ble_evt_local.msg.ble_tx.data = ble_evt_local.msg.ble_rx.data;
        ble_tx_len = ble_evt_local.msg.ble_tx.data_len = ble_evt_local.msg.ble_rx.data_len;
        lwevt_dispatch_ex(&ble_evt_local, LWEVT_TYPE_EXT_BLE_TX); // 用本地句柄分发事件
        break;
    }
    case LWEVT_TYPE_EXT_BLE_TX:
    {
        // 使用DMA将接收到的数据发送回去
        HAL_UART_Transmit(&huart3, ble_evt_local.msg.ble_tx.data, ble_evt_local.msg.ble_tx.data_len, 1000);
        memset(ble_rx_buffer, 0, sizeof(ble_rx_buffer));
        memset(ble_tx_buffer, 0, sizeof(ble_tx_buffer));
        ble_rx_len = ble_tx_len = 0;
        // 重新启动接收，使用Ex函数，接收不定长数据
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, ble_rx_buffer, sizeof(ble_rx_buffer));
        // 关闭DMA传输过半中断（HAL库默认开启，但我们只需要接收完成中断）
        __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
        break;
    }
    default:
        break;
    }
}

void ble_msg_queue_init(void)
{
    lwevt_t *evt;

    // 注册一个用户事件监听器
    lwevt_register(ble_evt_callback);

    evt = lwevt_get_handle(); // 获取默认事件句柄

    /*
     * 使用全局句柄发送事件
     * 在多线程环境中必须确保线程安全
     evt->msg.ext1.par1 = 1;
     evt->msg.ext1.par2 = 2;
     lwevt_dispatch(LWEVT_TYPE_BLE_CONNECT_BASIC); // 分发事件，所有监听器都会收到
     */

    /*
     * 获取事件句柄，设置事件数据并分发事件 发送基本事件 - 不带任何数据
     lwevt_dispatch(LWEVT_TYPE_BLE_CONNECT_BASIC);
     */

    /*
     * 使用本地句柄发送事件 无需确保线程安全
     ble_evt_local.msg.ble_rx.data = ble_rx_buffer;
     ble_evt_local.msg.ble_rx.data_len = blr_rx_len;
     lwevt_dispatch_ex(&ble_evt_local, LWEVT_TYPE_EXT_BLE_RX); // 用本地句柄分发事件
     */
}

void ble_init(void)
{
    ble_msg_queue_init();

    multiTimerStart(&ble_timer, 10, ble_ticks_callback, NULL);

    // 使用Ex函数，接收不定长数据
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, ble_rx_buffer, sizeof(ble_rx_buffer));
    // 关闭DMA传输过半中断（HAL库默认开启，但我们只需要接收完成中断）
    __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
}
