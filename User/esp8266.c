#define LOG_TAG "ESP8266"
#include "esp8266.h"
#include "elog.h"

// https://www.cnblogs.com/yychuyu/articles/17895845.html

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
    ESP8266_STA_AP_MODE,
} ESP8266_Mode;

typedef enum
{
    ESP8266_STATE_IDLE = 0,
    ESP8266_STATE_RESET,
    ESP8266_STATE_INIT,
    ESP8266_STATE_CONNECTED,
    ESP8266_STATE_ERROR,
} ESP8266_State;

uint8_t esp8266_dma_rx_buffer[ESP_RX_LEN] __attribute__((aligned(4)));
#define ESP8266_RESET_EVENT_BIT (1 << 0)
#define ESP8266_SET_MODE_EVENT_BIT (1 << 1)
#define ESP8266_INIT_EVENT_BIT (1 << 2)
#define ESP8266_CONNECTED_EVENT_BIT (1 << 3)
uint32_t esp8266_notyfy_value = 0;

// https://docs.espressif.com/projects/esp-at/zh-cn/release-v2.3.0.0_esp8266/AT_Command_Set/index.html
const uint8_t esp_sta_mode[] = "AT+CWMODE=1\r\n";                                    // 设置为 station 模式
const uint8_t esp_ap_mode[] = "AT+CWMODE=2\r\n";                                     // 设置为 softAP 模式
const uint8_t esp_ap_sta_mode[] = "AT+CWMODE=3\r\n";                                 // 设置为 softAP+station 共存模式
const uint8_t esp_reset[] = "AT+RST\r\n";                                            // 重启 ESP8266
const uint8_t esp_connect[] = "AT+CWJAP=\"LAPTOP-V9C029E4\",\"83B9i4/9\"\r\n";       // 连接 WiFi
const uint8_t esp_get_ip[] = "AT+CIFSR\r\n";                                         // 获取 IP 地址
const uint8_t esp_start_tcp[] = "AT+CIPSTART=\"TCP\",\"192.168.31.214\",8080\r\n";   // 连接 TCP服务器
const uint8_t esp_set_single_connection[] = "AT+CIPMUX=0\r\n";                       // 单连接
const uint8_t esp_set_multi_connection[] = "AT+CIPMUX=1\r\n";                        // 多连接模式
const uint8_t esp_set_server[] = "AT+CIPSERVER=1,8080\r\n";                          // 开启 SERVER 模式，设置端口为 8080
const uint8_t esp_close_tcp[] = "AT+CIPCLOSE\r\n";                                   // 关闭 TCP 连接
const uint8_t esp_connect_tcp[] = "AT+CIPSTART=\"TCP\",\"192.168.31.214\",8080\r\n"; // 建立 TCP 连接到”192.168.X.XXX”,8080
const uint8_t esp_connect_udp[] = "AT+CIPSTART=\"UDP\",\"192.168.31.214\",8080\r\n"; // 建立 UDP 连接到”192.168.X.XXX”,8080
const uint8_t esp_disconnect_ap[] = "AT+CWQAP\r\n";                                  // 断开热点
const uint8_t esp_send_data[] = "AT+CIPSEND=n\r\n";                                  // 开始传输，n表示需要传输的字节数
const uint8_t esp_set_transparent_mode[] = "AT+CIPMODE=1\r\n";                       // 开启透传模式
const uint8_t esp_exit_transparent_mode[] = "AT+CIPMODE=0\r\n";                      // 退出透传模式
const uint8_t esp_query_ip[] = "AT+CIPSTA?\r\n";                                     // 查询 ESP8266 的 IP 、网关地址和子网掩码
const uint8_t esp_query_cmd[] = "AT+CMD?\r\n";                                       // 查询当前固件支持的所有命令及命令类型
#define esp_delay delay_ms(4000)

typedef struct
{
    ESP8266_State state;
    ESP8266_Mode mode;    // 工作模式
    const char *ssid;     // WiFi名称
    const char *password; // WiFi密码
    uint8_t tcp_ip[4];    // 服务器服务端IP 192.168.31.214
    uint16_t tcp_port;    // 服务器端口 8080
    uint8_t cip_mux;      // 0:单连接 1:多连接
} ESP8266_HandleTypeDef;

ESP8266_HandleTypeDef esp8266 = {ESP8266_STATE_IDLE, 0};

void esp8266_enable(void)
{
    HAL_GPIO_WritePin(ESP8266_EN_GPIO_Port, ESP8266_EN_Pin, GPIO_PIN_SET);
}

void esp8266_hw_init(void)
{
    esp8266.mode = ESP8266_STA_AP_MODE;
    esp8266.ssid = "LAPTOP-V9C029E4";
    esp8266.password = "83B9i4/9";
    esp8266.tcp_ip[0] = 192;
    esp8266.tcp_ip[1] = 168;
    esp8266.tcp_ip[2] = 31;
    esp8266.tcp_ip[3] = 214;
    esp8266.tcp_port = 8080;
    esp8266.cip_mux = 0; // 单连接

    // 使用Ex函数，接收不定长数据
    HAL_UARTEx_ReceiveToIdle_DMA(&huart4, esp8266_dma_rx_buffer, sizeof(esp8266_dma_rx_buffer));
    __HAL_DMA_DISABLE_IT(huart4.hdmarx, DMA_IT_HT);
}

// esp8266_enable();
// esp_delay;
// HAL_UART_Transmit(&huart4, esp_ap_sta_mode, sizeof(esp_ap_sta_mode) - 1, 1000);
// esp_delay;
// HAL_UART_Transmit(&huart4, esp_reset, sizeof(esp_reset) - 1, 1000);
// esp_delay;
// HAL_UART_Transmit(&huart4, esp_connect, sizeof(esp_connect) - 1, 5000);
// esp_delay;
// esp_delay;
// HAL_UART_Transmit(&huart4, esp_get_ip, sizeof(esp_get_ip) - 1, 1000);
// esp_delay;
void wifi_rx_event_callback(uint16_t Size)
{
    log_i("%s", esp8266_dma_rx_buffer);
    memset(esp8266_dma_rx_buffer, 0, sizeof(esp8266_dma_rx_buffer));
}

uint8_t esp8266_send_command(const char *cmd)
{
    HAL_UART_Transmit(&huart4, (uint8_t *)cmd, strlen(cmd), 1000);
    return ESP8266_OK;
}

uint8_t esp8266_set_mode(ESP8266_Mode mode)
{
    switch (mode)
    {
    case ESP8266_STA_MODE:
        return esp8266_send_command("AT+CWMODE=1\r\n"); /* Station模式 */

    case ESP8266_AP_MODE:
        return esp8266_send_command("AT+CWMODE=2\r\n"); /* AP模式 */

    case ESP8266_STA_AP_MODE:
        return esp8266_send_command("AT+CWMODE=3\r\n"); /* AP+Station模式 */
    }
    return ESP8266_ERROR;
}

void esp8266_process_handler(void)
{
    static uint8_t sub_state = 0;
    switch (esp8266.state)
    {
    case ESP8266_STATE_IDLE:
    {
        esp8266_enable();
        vTaskDelay(pdMS_TO_TICKS(1000));
        esp8266.state = ESP8266_STATE_RESET;
    }
    break;
    case ESP8266_STATE_RESET:
    {
        if (sub_state == 0)
        {
            esp8266_send_command((char *)esp_reset);
            log_i("ESP8266 resetting...");
            sub_state++;
        }
        else
        {
            /**
             * @brief Wait for ESP8266 reset event
             * @param ulBitsToClearOnEntry 进入该函数时要清除的位
             * @param ulBitsToClearOnExit 退出该函数时要清除的位
             * @param pulNotificationValue 返回的通知值
             * @param xTicksToWait 等待的最大时间
             */
            xTaskNotifyWait(0x00, 0xFFFFFFFF, &esp8266_notyfy_value, portMAX_DELAY);
            if (esp8266_notyfy_value & ESP8266_RESET_EVENT_BIT)
            {
                log_i("ESP8266 reset complete");
                esp8266.state = ESP8266_STATE_INIT;
            }
        }
    }
    break;
    case ESP8266_STATE_INIT:
    {
        esp8266_set_mode(esp8266.mode);
    }
    break;
    default:
        break;
    }
}
