#define LOG_TAG "app"
#include "app.h"
#include "elog.h"
#include "lvgl.h"

#define START_TASK_STACK_SIZE 128
#define START_TASK_PRIORITY 15
TaskHandle_t start_task_handle = NULL;
void start_task(void *pvParameters);

#define APP_TASK_STACK_SIZE 512
#define APP_TASK_PRIORITY 10 // �����ȼ� Խ��Խ��
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

// ��ť����¼��ص�����
static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *btn = lv_event_get_target(e);

    if (code == LV_EVENT_CLICKED)
    {
        static uint8_t cnt = 0;
        cnt++;

        // ��ȡ��ť�ı�ǩ���󲢸����ı�
        lv_obj_t *label = lv_obj_get_child(btn, 0);
        lv_label_set_text_fmt(label, "Clicked: %d", cnt);

        // ����������������������Ĳ���
        // ����ı䰴ť��ɫ��������Ϣ��
    }
}

void create_clickable_button(void)
{
    // ����һ����ť
    lv_obj_t *btn = lv_btn_create(lv_scr_act());

    // ���ð�ťλ�úʹ�С
    lv_obj_set_size(btn, 120, 50);              // ��� 120, �߶� 50
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, 0, 0); // ������ʾ

    // Ϊ��ť��ӱ�ǩ
    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, "Click Me!");
    lv_obj_center(label);

    // ���ð�ť��ʽ����ѡ��
    static lv_style_t style_btn;
    lv_style_init(&style_btn);

    // ����״̬��ʽ
    lv_style_set_bg_color(&style_btn, lv_color_hex(0x007ac3));
    lv_style_set_bg_opa(&style_btn, LV_OPA_COVER);
    lv_style_set_radius(&style_btn, 10);
    lv_style_set_border_width(&style_btn, 0);

    // ����״̬��ʽ
    lv_style_set_bg_color(&style_btn, lv_color_hex(0x005a93));

    // Ӧ����ʽ
    lv_obj_add_style(btn, &style_btn, 0);

    // ���ñ�ǩ��ʽ����ѡ��
    static lv_style_t style_label;
    lv_style_init(&style_label);
    lv_style_set_text_color(&style_label, lv_color_white());
    lv_style_set_text_font(&style_label, &lv_font_montserrat_14);
    lv_obj_add_style(label, &style_label, 0);

    // ��ӵ���¼��ص�
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
    // lv_demo_stress(); /* ���Ե�demo */
    create_clickable_button();
    while (1)
    {
        lv_timer_handler(); /* LVGL��ʱ�� */
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

    lv_init();            /* lvglϵͳ��ʼ�� */
    lv_port_disp_init();  /* lvgl��ʾ�ӿڳ�ʼ��,����lv_init()�ĺ��� */
    lv_port_indev_init(); /* lvgl����ӿڳ�ʼ��,����lv_init()�ĺ��� */

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
