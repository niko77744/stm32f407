#define LOG_TAG "app"
#include "app.h"
#include "elog.h"
#include "lvgl.h"
#include "shell.h"
#include "shell_port.h"
#include "ff.h"
#include "ffconf.h"
#include "fal.h"
#include "tim.h"

#define START_TASK_STACK_SIZE (1024 / 4)
#define START_TASK_PRIORITY 15
TaskHandle_t start_task_handle = NULL;
void start_task(void *pvParameters);

#define APP_TASK_STACK_SIZE (1024 / 4)
#define APP_TASK_PRIORITY 5 // 低优先级 越大越高
TaskHandle_t app_task_handle = NULL;
void app_task(void *pvParameters);

#define DISPLAY_TASK_STACK_SIZE (8192 / 4)
#define DISPLAY_TASK_PRIORITY 5
TaskHandle_t display_task_handle = NULL;
void display_task(void *pvParameters);

#define SHELL_TASK_STACK_SIZE (1024 / 4)
#define SHELL_TASK_PRIORITY 5
TaskHandle_t shell_task_handle = NULL;

// static SemaphoreHandle_t Fatfs_Mutex_Semaphore = NULL;

void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();
    // Fatfs_Mutex_Semaphore = xSemaphoreCreateMutex();
#if SUPPORT_SHELL == 1
    userShellInit();
#endif
    xTaskCreate(
        (TaskFunction_t)app_task,
        (char *)"app_task",
        APP_TASK_STACK_SIZE,
        NULL,
        APP_TASK_PRIORITY,
        &app_task_handle);
    xTaskCreate(
        (TaskFunction_t)display_task,
        (char *)"display_task",
        DISPLAY_TASK_STACK_SIZE,
        NULL,
        DISPLAY_TASK_PRIORITY,
        &display_task_handle);
#if SUPPORT_SHELL == 1
    xTaskCreate(
        (TaskFunction_t)shellTask,
        (char *)"shell",
        SHELL_TASK_STACK_SIZE,
        &shell,
        SHELL_TASK_PRIORITY,
        &shell_task_handle);
#endif
    vTaskDelete(NULL);
    taskEXIT_CRITICAL();
}

void app_task(void *pvParameters)
{
    while (1)
    {
        sw_timer_loop();
        iap_uart_proceess();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void display_task(void *pvParameters)
{
#if SUPPORT_LVGL == 1
    create_jump_button();
    create_erase_button();
    // lv_demo_stress(); /* 测试的demo */
    // lv_demo_music();  /* 测试的demo */
#endif
    while (1)
    {
#if SUPPORT_LVGL == 1
        lv_timer_handler(); /* LVGL计时器 */
#endif
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// static FIL file; // _FS_TINY 在Normal模式下，每个FIL对象都会包含一个缓冲区（大小为_MAX_SS，通常为512字节），因此如果我们在任务栈上定义FIL对象，会占用大量栈空间（至少512字节加上其他局部变量）。而使用静态分配，将FIL对象放在全局数据区，就不会占用任务栈空间。
// void iap_task(void *pvParameters)
// {
//     FRESULT res;
//     UINT byteswritten;
//     const char *text = "Hello, FatFS! This is a test file.";
//     while (1)
//     {
//         if (xSemaphoreTake(Fatfs_Mutex_Semaphore, portMAX_DELAY) == pdTRUE)
//         {
//             sd_fatfs_self_inspection();
//             res = f_open(&file, "test.txt", FA_WRITE | FA_CREATE_ALWAYS);
//             if (res != FR_OK)
//             {
//                 log_e("Error creating file: %d", res);
//             }
//             log_i("file opened successfully");
//             res = f_write(&file, text, strlen(text), &byteswritten);
//             if (res != FR_OK || byteswritten != strlen(text))
//             {
//                 log_e("Error writing to file: %d", res);
//             }
//             res = f_close(&file);
//             if (res != FR_OK)
//             {
//                 log_e("Error closing file: %d", res);
//             }
//             log_i("file closed successfully");
//             xSemaphoreGive(Fatfs_Mutex_Semaphore);
//         }
//         vTaskDelay(pdMS_TO_TICKS(3000));
//     }
// }
void app_init(void)
{
    Memory_Init(INSRAM);
    sw_time_init();
#if SUPPORT_LOG == 1
    log_init();
#endif
    fal_init();
    sd_fatfs_init();
    // user_lfs_init();

    sys_time_init();
    buttons_init();
    app_led_init();
    iap_init();

    __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC1 | TIM_IT_CC2);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);

    // ble_init();
    // iap_init(iap_from_uart);
    // esp8266_hw_init();
    // iap_process(); // 先擦除 stmflash_earse(FLASH_APP1_ADDR, 0x1000); // 4096
#if SUPPORT_LVGL == 1
    lv_init();            /* lvgl系统初始化 */
    lv_port_disp_init();  /* lvgl显示接口初始化,放在lv_init()的后面 */
    lv_port_indev_init(); /* lvgl输入接口初始化,放在lv_init()的后面 */
#endif
}

void app_run(void)
{
#if SUPPORT_OS == 0
    while (1)
    {
        sw_timer_loop();
    }
#else
    xTaskCreate(
        (TaskFunction_t)start_task,
        "start_task",
        START_TASK_STACK_SIZE,
        NULL,
        START_TASK_PRIORITY,
        &start_task_handle);
    vTaskStartScheduler();
#endif
}

// 不定长数据接收完成回调函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        extern DMA_HandleTypeDef hdma_usart1_rx;
        // switch (huart->RxEventType)
        // {
        // case HAL_UART_RXEVENT_TC:
        //     break;
        // case HAL_UART_RXEVENT_HT:
        //     break;
        // case HAL_UART_RXEVENT_IDLE:
        //     break;
        // }
        iap_rx_event_callback(Size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, iap_dma_rx_buf, sizeof(iap_dma_rx_buf));
        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    }
    else if (huart->Instance == USART3) // ble
    {
        ble_rx_event_callback(Size);
    }
    else if (huart->Instance == UART4) // wifi
    {
        wifi_rx_event_callback(Size);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART6) // shell
    {
        shell_recv_byte();
    }
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART3) // ble
    {
        log_i("BLE data send over");
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4) // wifi
    {
        __HAL_UART_CLEAR_FEFLAG(huart);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart4, esp8266_buf, sizeof(esp8266_buf));
    }
    else if (huart->Instance == USART1)
    {
        __HAL_UART_CLEAR_FEFLAG(huart);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, iap_dma_rx_buf, sizeof(iap_dma_rx_buf));
    }
}

// 1代表100us
#define RF433_MS(n) ((uint32_t)((n) * 10.0f))
// 时间检查辅助宏
#define TIME_IN_RANGE(t, min, max) ((t) >= (min) && (t) <= (max))
#define NEC_DATA_BITS_TOTAL (32) // NEC协议总位数
#define NEC_BITS_PER_BYTE (8)    // 每字节位数
// 接收状态
typedef enum
{
    STATE_WAIT_LEADER = 0,
    STATE_RECEIVE_DATA,
    STATE_COMPLETE
} nec_recv_state_e;

// 接收数据结构
typedef struct
{
    uint8_t addr;
    uint8_t cmd;
} nec_data_t;

// 全局变量
struct
{
    nec_recv_state_e state;
    uint8_t bit_cnt;
    uint32_t raw_data;
    uint32_t low_time;  // 低电平时间
    uint32_t high_time; // 高电平时间
    nec_data_t result;
} nec_recv = {0};

// NEC数据校验
static uint8_t nec_check(uint8_t addr, uint8_t addr_inv, uint8_t cmd, uint8_t cmd_inv)
{
    return ((uint8_t)~addr == addr_inv) && ((uint8_t)~cmd == cmd_inv);
}

// 中断回调函数 - 简洁版
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM1)
        return;

    // 上升沿捕获（低电平结束）
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        nec_recv.low_time = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        __HAL_TIM_SET_COUNTER(htim, 0);
    }
    // 下降沿捕获（高电平结束）
    else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        nec_recv.high_time = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        __HAL_TIM_SET_COUNTER(htim, 0);

        switch (nec_recv.state)
        {
        case STATE_WAIT_LEADER:
            // 检查引导码
            if (TIME_IN_RANGE(nec_recv.low_time, RF433_MS(8.0f), RF433_MS(10.0f)) &&
                TIME_IN_RANGE(nec_recv.high_time, RF433_MS(4.0f), RF433_MS(5.0f)))
            {
                nec_recv.state = STATE_RECEIVE_DATA;
                nec_recv.bit_cnt = 0;
                nec_recv.raw_data = 0;
            }
            break;

        case STATE_RECEIVE_DATA:
            if (!TIME_IN_RANGE(nec_recv.low_time, RF433_MS(0.4f), RF433_MS(0.9f)) ||
                !TIME_IN_RANGE(nec_recv.high_time, RF433_MS(0.4f), RF433_MS(1.9f)))
            {
                nec_recv.state = STATE_COMPLETE;
                break;
            }

            // 解析数据位
            nec_recv.raw_data <<= 1;
            if (nec_recv.high_time >= RF433_MS(1.0f))
            {
                nec_recv.raw_data |= 1; // 逻辑1
            }

            nec_recv.bit_cnt++;
            // 检查是否接收完32位
            if (nec_recv.bit_cnt >= NEC_DATA_BITS_TOTAL)
            {
                nec_recv.state = STATE_COMPLETE;
            }
            break;
        }

        // 接收完成，解析数据
        if (nec_recv.state == STATE_COMPLETE)
        {
            // 按接收顺序：地址->地址反码->命令->命令反码
            uint8_t *p = (uint8_t *)&nec_recv.raw_data;
            uint8_t addr = p[3];
            uint8_t addr_inv = p[2];
            uint8_t cmd = p[1];
            uint8_t cmd_inv = p[0];

            uint8_t result = nec_check(addr, addr_inv, cmd, cmd_inv);
            if (result)
            {
                nec_recv.result.addr = addr;
                nec_recv.result.cmd = cmd;
                beep_start(beep_short);
            }

            // 重置状态，等待下一帧
            nec_recv.state = STATE_WAIT_LEADER;
        }
    }
}
