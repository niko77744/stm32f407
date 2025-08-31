#define LOG_TAG "buzzer"
#include "buzzer.h"

typedef struct
{
    uint8_t state;
} buzzer_t;

MultiTimer buzzer_timer;
buzzer_t buzzer_handle = {0};

void buzzer_on(void)
{
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_SET);
    buzzer_handle.state = 1;
}

void buzzer_off(void)
{
    HAL_GPIO_WritePin(Buzzer_GPIO_Port, Buzzer_Pin, GPIO_PIN_RESET);
    buzzer_handle.state = 0;
}

static void buzzer_callback(MultiTimer *timer, void *userData)
{
    buzzer_off();
}

void beep_start(buzzer_mode_e mode)
{
    if (buzzer_handle.state == 1)
        return;

    buzzer_on();
    switch (mode)
    {
    case beep_short:
        multiTimerStart(&buzzer_timer, BEEP_SHORT_TIME, buzzer_callback, NULL);
        break;
    case beep_long:
        multiTimerStart(&buzzer_timer, BEEP_LONG_TIME, buzzer_callback, NULL);
        break;
    default:
        buzzer_off();
        break;
    }
}
