#define LOG_TAG "ESP8266"
#include "esp8266.h"
#include "elog.h"

// https://www.cnblogs.com/yychuyu/articles/17895845.html

uint8_t esp8266_buf[256];

uint8_t esp_mode[] = "AT+CWMODE=3\r\n";                                     // 设置为 softAP+station 共存模式
uint8_t esp_reset[] = "AT+RST\r\n";                                         // 重启 ESP8266
uint8_t esp_connect[] = "AT+CWJAP=\"LAPTOP-V9C029E4\",\"83B9i4/9\"\r\n";    // 连接 WiFi
uint8_t esp_get_ip[] = "AT+CIFSR\r\n";                                      // 获取 IP 地址
uint8_t esp_start_tcp[] = "AT+CIPSTART=\"TCP\",\"192.168.3.116\",8080\r\n"; // 连接 TCP服务器
#define esp_delay HAL_Delay(4000)
void esp8266_enable(void)
{
    HAL_GPIO_WritePin(ESP8266_EN_GPIO_Port, ESP8266_EN_Pin, GPIO_PIN_SET);
}

void esp8266_hw_init(void)
{
    // 使用Ex函数，接收不定长数据
    HAL_UARTEx_ReceiveToIdle_IT(&huart4, esp8266_buf, sizeof(esp8266_buf));

    esp8266_enable();
    esp_delay;
    HAL_UART_Transmit(&huart4, esp_mode, sizeof(esp_mode) - 1, 1000);
    esp_delay;
    // HAL_UART_Transmit(&huart4, esp_reset, sizeof(esp_reset) - 1, 1000);
    // esp_delay;
    HAL_UART_Transmit(&huart4, esp_connect, sizeof(esp_connect) - 1, 5000);
    esp_delay;
    esp_delay;
    HAL_UART_Transmit(&huart4, esp_get_ip, sizeof(esp_get_ip) - 1, 1000);
    esp_delay;
    // esp_delay;
    // HAL_UART_Transmit(&huart4, esp_start_tcp, sizeof(esp_start_tcp), 1000);
    // esp_delay;
}

typedef enum
{
    ESP8266_OK = 0,
    ESP8266_ERROR,
    ESP8266_TIMEOUT,
} ESP8266_Status;

typedef enum
{
    ESP8266_STA_MODE = 1,
    ESP8266_AP_MODE,
    ESP8266_STA_AP_MODE
} ESP8266_Mode;

ESP8266_Status esp8266_send_command(const char *cmd, const char *ack)
{
    uint8_t recv_buf[128] = {0};
    HAL_UART_Transmit(&huart4, (uint8_t *)cmd, strlen(cmd), 1000);
    HAL_Delay(500);
    HAL_UART_Receive(&huart4, recv_buf, sizeof(recv_buf) - 1, 1000);
    if (strstr((char *)recv_buf, ack) != NULL)
    {
        log_i("ESP8266 command success: %s", cmd);
        return ESP8266_OK;
    }
    else
    {
        log_e("ESP8266 command failed: %s", cmd);
        return ESP8266_ERROR;
    }
}

uint8_t esp8266_set_mode(ESP8266_Mode mode)
{
    switch (mode)
    {
    case ESP8266_STA_MODE:
        return esp8266_send_command("AT+CWMODE=1\r\n", "OK"); /* Station模式 */

    case ESP8266_AP_MODE:
        return esp8266_send_command("AT+CWMODE=2\r\n", "OK"); /* AP模式 */

    case ESP8266_STA_AP_MODE:
        return esp8266_send_command("AT+CWMODE=3\r\n", "OK"); /* AP+Station模式 */
    }
    return ESP8266_ERROR;
}

uint8_t esp8266_single_connection(void)
{
    return esp8266_send_command("AT+CIPMUX=0\r\n", "OK");
}

uint8_t esp8266_join_ap(char *ssid, char *pwd)
{
    char cmd[64];
    sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
    return esp8266_send_command(cmd, "WIFI GOT IP");
}

uint8_t esp8266_get_ip(char *buf)
{
    char *p_start;
    char *p_end;

    if (esp8266_send_command("AT+CIFSR\r\n", "STAIP") != ESP8266_OK)
        return ESP8266_ERROR;

    p_start = strstr((char *)esp8266_buf, "\"");
    p_end = strstr(p_start + 1, "\"");
    *p_end = '\0';
    sprintf(buf, "%s", p_start + 1);

    return ESP8266_OK;
}

uint8_t esp8266_connect_tcp_server(char *server_ip, char *server_port)
{
    char cmd[64];
    sprintf(cmd, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", server_ip, server_port);
    return esp8266_send_command(cmd, "CONNECT");
}
