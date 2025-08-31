#define LOG_TAG "led"
#include "led.h"

void led_on(LED_TypeDef led)
{
    switch (led)
    {
    case LED0:
        HAL_GPIO_WritePin(LED_0_GPIO_Port, LED_0_Pin, GPIO_PIN_RESET);
        break;
    case LED1:
        HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_RESET);
        break;
    case LED2:
        HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_RESET);
        break;
    default:
        break;
    }
}

void led_off(LED_TypeDef led)
{
    switch (led)
    {
    case LED0:
        HAL_GPIO_WritePin(LED_0_GPIO_Port, LED_0_Pin, GPIO_PIN_SET);
        break;
    case LED1:
        HAL_GPIO_WritePin(LED_1_GPIO_Port, LED_1_Pin, GPIO_PIN_SET);
        break;
    case LED2:
        HAL_GPIO_WritePin(LED_2_GPIO_Port, LED_2_Pin, GPIO_PIN_SET);
        break;
    default:
        break;
    }
}
