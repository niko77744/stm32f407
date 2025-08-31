#define LOG_TAG "BLE"
#include "ble.h"
#include "elog.h"

MultiTimer ble_timer;

GPIO_PinState get_ble_device_state(void)
{
    return HAL_GPIO_ReadPin(BLE_Connect_GPIO_Port, BLE_Connect_Pin);
}

void ble_connect_callback(void)
{
    log_i("BLE connected");
}

void ble_disconnect_callback(void)
{
    log_i("BLE disconnected");
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
    multiTimerStart(&ble_timer, 500, button_ticks_callback, NULL);
}

void ble_init(void)
{
    multiTimerStart(&ble_timer, 500, button_ticks_callback, NULL);
}
