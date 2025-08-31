#define LOG_TAG "BLE"
#include "ble.h"
#include "elog.h"

#define BLE_RX_BUFFER_SIZE 256
uint8_t ble_rx_buffer[BLE_RX_BUFFER_SIZE] = {0};
MultiTimer ble_timer;

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

static void button_ticks_callback(MultiTimer *timer, void *userData)
{
    static uint8_t prev_state = 0;
    uint8_t curr_state = get_ble_device_state();

    if (curr_state != prev_state)
    {
        if (curr_state == GPIO_PIN_SET)
            ble_connect_callback();
        else
            ble_disconnect_callback();
        prev_state = curr_state;
    }
    multiTimerStart(&ble_timer, 10, button_ticks_callback, NULL);
}

// ���������ݽ�����ɻص�����
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART3)
    {
        // ʹ��DMA�����յ������ݷ��ͻ�ȥ
        // HAL_UART_Transmit_DMA(&huart3, ble_rx_buffer, Size);
        HAL_UART_Transmit(&huart3, ble_rx_buffer, Size, 1000);
        memset(ble_rx_buffer, 0, sizeof(ble_rx_buffer));
        // �����������գ�ʹ��Ex���������ղ���������
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, ble_rx_buffer, sizeof(ble_rx_buffer));
        // �ر�DMA��������жϣ�HAL��Ĭ�Ͽ�����������ֻ��Ҫ��������жϣ�
        __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3)
    {
        log_i("BLE data send over");
    }
}

void ble_init(void)
{
    multiTimerStart(&ble_timer, 10, button_ticks_callback, NULL);

    // ʹ��Ex���������ղ���������
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, ble_rx_buffer, sizeof(ble_rx_buffer));
    // �ر�DMA��������жϣ�HAL��Ĭ�Ͽ�����������ֻ��Ҫ��������жϣ�
    __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
}
