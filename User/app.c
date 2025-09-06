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

#define DISPLAY_TASK_STACK_SIZE 4096
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

// 按钮点击事件回调函数
static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED)
    {
        static uint8_t cnt = 0;
        cnt++;

        // 获取按钮的标签对象并更新文本
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Clicked: %d", cnt);

        // 可以在这里添加其他点击后的操作
        // 例如改变按钮颜色、发送消息等
    }
}

void create_clickable_button(void)
{
    // 创建一个按钮
    lv_obj_t *btn = lv_btn_create(lv_scr_act());

    // 设置按钮位置和大小
    lv_obj_set_size(btn, 120, 50);              // 宽度 120, 高度 50
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 0, 0); // 居中显示

    // 为按钮添加标签
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Click Me!");
    lv_obj_center(label);

    // 设置按钮样式（可选）
    static lv_style_t style_btn;
    lv_style_init(&style_btn);

    // 正常状态样式
    lv_style_set_bg_color(&style_btn, lv_color_hex(0x007ac3));
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_radius(&style_btn, 10);
    lv_style_set_border_width(&style_btn, 0);

    // 按下状态样式
    lv_style_set_bg_color(&style_btn, lv_color_hex(0x005a93));

    // 应用样式
    lv_obj_add_style(btn, &style_btn, 0);

    // 设置标签样式（可选）
    static lv_style_t style_label;
    lv_style_init(&style_label);
    lv_style_set_text_color(&style_label, lv_color_white());
    lv_style_set_text_font(&style_label, &lv_font_montserrat_14);
    lv_obj_add_style(label, &style_label, 0);

    // 添加点击事件回调
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);
}

void app_task(void *pvParameters)
{
    while (1)
    {
        sw_timer_loop();
        led_toggle(LED0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void display_task(void *pvParameters)
{
    // lv_demo_stress(); /* 测试的demo */
    create_clickable_button();
    while (1)
    {
        lv_timer_handler(); /* LVGL计时器 */
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void communication_task(void *pvParameters)
{
    while (1)
    {
        log_i("communication task is running.");
        led_toggle(LED1);
        led_toggle(LED2);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_init(void)
{
    Memory_Init(INSRAM);
    log_init();
    // user_lfs_init();
    // nvs_flash_init();
    // sw_time_init();
    // sys_time_init();
    buttons_init();
    ble_init();

    lv_init();            /* lvgl系统初始化 */
    lv_port_disp_init();  /* lvgl显示接口初始化,放在lv_init()的后面 */
    lv_port_indev_init(); /* lvgl输入接口初始化,放在lv_init()的后面 */

    // sd_fatfs_init();
    // ring_buf_init();
    // esp8266_hw_init();
    // message_queue_init();
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
