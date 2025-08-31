#define LOG_TAG "ESP8266"
#include "esp8266.h"
#include "elog.h"
void esp8266_enable(void)
{
    HAL_GPIO_WritePin(ESP8266_EN_GPIO_Port, ESP8266_EN_Pin, GPIO_PIN_SET);
}

void esp8266_hw_init(void)
{
    // MX_UART4_Init();
    esp8266_enable();
}
