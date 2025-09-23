#define LOG_TAG "app"
#include "app.h"
#include "elog.h"
#include "lvgl.h"

#define START_TASK_STACK_SIZE 128
#define START_TASK_PRIORITY 15
TaskHandle_t start_task_handle = NULL;
void start_task(void *pvParameters);

#define APP_TASK_STACK_SIZE 512
#define APP_TASK_PRIORITY 10 // 低优先级 越大越高
TaskHandle_t app_task_handle = NULL;
void app_task(void *pvParameters);

#define DISPLAY_TASK_STACK_SIZE 8192
#define DISPLAY_TASK_PRIORITY 5
TaskHandle_t display_task_handle = NULL;
void display_task(void *pvParameters);

#define COMMUNICATION_TASK_STACK_SIZE 512
#define COMMUNICATION_TASK_PRIORITY 8
TaskHandle_t communication_task_handle = NULL;
void communication_task(void *pvParameters);

void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();
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
        (TaskFunction_t)communication_task,
        (char *)"communication_task",
        COMMUNICATION_TASK_STACK_SIZE,
        NULL,
        COMMUNICATION_TASK_PRIORITY,
        &communication_task_handle);

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
    // create_clickable_button();
    // lv_demo_stress(); /* 测试的demo */
    // lv_demo_music(); /* 测试的demo */
    while (1)
    {
        // lv_timer_handler(); /* LVGL计时器 */
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void communication_task(void *pvParameters)
{
    while (1)
    {
        led_toggle(LED0);
        led_toggle(LED1);
        led_toggle(LED2);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_init(void)
{
    Memory_Init(INSRAM);
    log_init();
    sd_fatfs_init();
    // user_lfs_init();
    // nvs_flash_init();

    // ring_buf_init();
    // sys_time_init();
    sw_time_init();
    message_queue_init();
    buttons_init();
    ble_init();
    // esp8266_hw_init();

    // lv_init();            /* lvgl系统初始化 */
    // lv_port_disp_init();  /* lvgl显示接口初始化,放在lv_init()的后面 */
    // lv_port_indev_init(); /* lvgl输入接口初始化,放在lv_init()的后面 */
}

void app_os_start(void)
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
