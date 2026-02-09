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
    userShellInit();
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
    xTaskCreate(
        (TaskFunction_t)shellTask,
        (char *)"shell",
        SHELL_TASK_STACK_SIZE,
        &shell,
        SHELL_TASK_PRIORITY,
        &shell_task_handle);
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
    log_init();
    fal_init();
    sd_fatfs_init();
    // user_lfs_init();

    sys_time_init();
    buttons_init();
    app_led_init();
    iap_init();
    ble_init();
    esp8266_hw_init();
    rf433_init();

#if SUPPORT_LVGL == 1
    lv_init();            /* lvgl系统初始化 */
    lv_port_disp_init();  /* lvgl显示接口初始化,放在lv_init()的后面 */
    lv_port_indev_init(); /* lvgl输入接口初始化,放在lv_init()的后面 */
#endif
}

void app_run(void)
{
    xTaskCreate(
        (TaskFunction_t)start_task,
        "start_task",
        START_TASK_STACK_SIZE,
        NULL,
        START_TASK_PRIORITY,
        &start_task_handle);
    vTaskStartScheduler();
}

// 不定长数据接收完成回调函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        // huart->RxEventType  HAL_UART_RXEVENT_TC  HAL_UART_RXEVENT_HT  HAL_UART_RXEVENT_IDLE
        extern DMA_HandleTypeDef hdma_usart1_rx;
        iap_rx_event_callback(Size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, iap_dma_rx_buffer, sizeof(iap_dma_rx_buffer));
        __HAL_DMA_DISABLE_IT(&hdma_usart1_rx, DMA_IT_HT);
    }
    else if (huart->Instance == USART3) // ble
    {
        ble_rx_event_callback(Size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, ble_dma_rx_buffer, sizeof(ble_dma_rx_buffer));
        __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT);
    }
    else if (huart->Instance == UART4) // wifi
    {
        wifi_rx_event_callback(Size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart4, esp8266_dma_rx_buffer, sizeof(esp8266_dma_rx_buffer));
        __HAL_DMA_DISABLE_IT(huart4.hdmarx, DMA_IT_HT);
    }
    else if (huart->Instance == USART6) // shell
    {
        extern DMA_HandleTypeDef hdma_usart6_rx;
        shell_rx_event_callback(Size);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart6, shell_dma_rx_buffer, sizeof(shell_dma_rx_buffer));
        __HAL_DMA_DISABLE_IT(&hdma_usart6_rx, DMA_IT_HT);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == UART4) // wifi
    {
        __HAL_UART_CLEAR_FEFLAG(huart);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart4, esp8266_dma_rx_buffer, sizeof(esp8266_dma_rx_buffer));
    }
    else if (huart->Instance == USART1)
    {
        __HAL_UART_CLEAR_FEFLAG(huart);
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, iap_dma_rx_buffer, sizeof(iap_dma_rx_buffer));
    }
}
