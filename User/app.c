#define LOG_TAG "app"
#include "app.h"
#include "elog.h"
#include "lvgl.h"
#include "shell.h"
#include "shell_port.h"
#include "fal_cfg.h"

#define START_TASK_STACK_SIZE 1024
#define START_TASK_PRIORITY 15
TaskHandle_t start_task_handle = NULL;
void start_task(void *pvParameters);

#define APP_TASK_STACK_SIZE 512
#define APP_TASK_PRIORITY 5 // 低优先级 越大越高
TaskHandle_t app_task_handle = NULL;
void app_task(void *pvParameters);

#define DISPLAY_TASK_STACK_SIZE 8192
#define DISPLAY_TASK_PRIORITY 5
TaskHandle_t display_task_handle = NULL;
void display_task(void *pvParameters);

#define COMMUNICATION_TASK_STACK_SIZE 512
#define COMMUNICATION_TASK_PRIORITY 5
TaskHandle_t communication_task_handle = NULL;
void communication_task(void *pvParameters);

#define IAP_TASK_STACK_SIZE 4096
#define IAP_TASK_PRIORITY 6
TaskHandle_t iap_task_handle = NULL;
void iap_task(void *pvParameters);

#define SHELL_TASK_STACK_SIZE 2048
#define SHELL_TASK_PRIORITY 5

static SemaphoreHandle_t Fatfs_Mutex_Semaphore = NULL;

void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();
    Fatfs_Mutex_Semaphore = xSemaphoreCreateMutex();
    userShellInit();
    xTaskCreate(
        (TaskFunction_t)app_task,
        (char *)"app_task",
        APP_TASK_STACK_SIZE,
        NULL,
        APP_TASK_PRIORITY,
        &app_task_handle);
    // xTaskCreate(
    //     (TaskFunction_t)display_task,
    //     (char *)"display_task",
    //     DISPLAY_TASK_STACK_SIZE,
    //     NULL,
    //     DISPLAY_TASK_PRIORITY,
    //     &display_task_handle);
    // xTaskCreate(
    //     (TaskFunction_t)communication_task,
    //     (char *)"communication_task",
    //     COMMUNICATION_TASK_STACK_SIZE,
    //     NULL,
    //     COMMUNICATION_TASK_PRIORITY,
    //     &communication_task_handle);
    // xTaskCreate(
    //     (TaskFunction_t)iap_task,
    //     (char *)"iap_task",
    //     IAP_TASK_STACK_SIZE,
    //     NULL,
    //     IAP_TASK_PRIORITY,
    //     &iap_task_handle);
    xTaskCreate(
        shellTask,
        "shell",
        SHELL_TASK_STACK_SIZE,
        &shell,
        SHELL_TASK_PRIORITY,
        NULL);
    vTaskDelete(NULL);
    taskEXIT_CRITICAL();
}

void app_task(void *pvParameters)
{
    while (1)
    {
        sw_timer_loop();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void display_task(void *pvParameters)
{
#if SUPPORT_LVGL == 1
    create_clickable_button();
    // lv_demo_stress(); /* 测试的demo */
    // lv_demo_music();  /* 测试的demo */
    while (1)
    {
        lv_timer_handler(); /* LVGL计时器 */
        vTaskDelay(pdMS_TO_TICKS(5));
    }
#else
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5));
    }
#endif
}

void communication_task(void *pvParameters)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void iap_task(void *pvParameters)
{
    while (1)
    {
        xSemaphoreTake(Fatfs_Mutex_Semaphore, portMAX_DELAY);
        xSemaphoreGive(Fatfs_Mutex_Semaphore);
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

const struct fal_flash_dev *fal_little_fs;
const struct fal_partition *fal_little_fs_partition;
extern int fal_init(void);
extern const struct fal_flash_dev *fal_flash_device_find(const char *name);
extern const struct fal_partition *fal_partition_find(const char *name);
void app_init(void)
{
    Memory_Init(INSRAM);
    // sw_time_init();
    log_init();
    sd_fatfs_init();
    // nvs_flash_init();
    fal_init();
    // fal_little_fs = fal_flash_device_find("norflash0");
    // if (fal_little_fs == NULL)
    //     log_i("Error: Flash Device (norflash0) not found.");
    // fal_little_fs_partition = fal_partition_find(FAL_LFS_PART_NAME);
    // if (fal_little_fs_partition == NULL)
    //     log_i("Error: Partition (%s) not found.", FAL_LFS_PART_NAME);
    // user_lfs_init();

    // ring_buf_init();
    // message_queue_init();
    // buttons_init();
    // app_led_init();
    // iap_init(iap_from_uart);
    // ble_init();
    // esp8266_hw_init();
    // stmflash_earse(FLASH_APP1_ADDR, 0x1000); // 4096
    // iap_process();
    // sys_time_init();
#if SUPPORT_LVGL == 1
    lv_init();            /* lvgl系统初始化 */
    lv_port_disp_init();  /* lvgl显示接口初始化,放在lv_init()的后面 */
    lv_port_indev_init(); /* lvgl输入接口初始化,放在lv_init()的后面 */
#endif
}

void app_os_start(void)
{
#if SUPPORT_OS == 0
    while (1)
    {
        sw_timer_loop();
        // iap_uart_proceess();
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
    if (huart->Instance == USART3) // ble
    {
        ble_rx_event_callback(Size);
    }
    else if (huart->Instance == UART4) // wifi
    {
        wifi_rx_event_callback(Size);
    }
    else if (huart->Instance == USART6) // log
    {
        // log_rx_event_callback(Size);
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
}
