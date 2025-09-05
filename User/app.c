#define LOG_TAG "app"
#include "app.h"
#include "elog.h"

#define START_TASK_STACK_SIZE 128
#define START_TASK_PRIORITY 15
TaskHandle_t start_task_handle = NULL;
void start_task(void *pvParameters);

#define APP_TASK_STACK_SIZE 512
#define APP_TASK_PRIORITY 10 // 低优先级 越大越高
TaskHandle_t app_task_handle = NULL;
void app_task(void *pvParameters);

#define DISPLAY_TASK_STACK_SIZE 512
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
        log_i("app task is running.");
        sw_timer_loop();
        led_toggle(LED0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// 清空屏幕
void Clear_Screen(void)
{
    LCD_Clear(WHITE);                                                // 清屏
    BRUSH_COLOR = BLUE;                                              // 设置字体为蓝色
    LCD_DisplayString(lcd_width - 40, lcd_height - 18, 16, "Clear"); // 显示清屏区域
    BRUSH_COLOR = RED;                                               // 设置画笔蓝色
}
void Draw_Point(uint16_t x, uint16_t y, uint16_t color)
{
    BRUSH_COLOR = color;
    for (uint8_t i = 0; i < 4; i++)
    {
        LCD_DrawPoint(x, y + i);
        LCD_DrawPoint(x + 1, y + i);
        LCD_DrawPoint(x + 2, y + i);
        LCD_DrawPoint(x + 3, y + i);
    }
}

void display_task(void *pvParameters)
{
    while (1)
    {
        // log_i("display task is running.");
        // led_toggle(LED1);
        XPT2046_Scan(0);
        if (Xdown < lcd_width && Ydown < lcd_height)
        {
            if (Xdown > (lcd_width - 40) && Ydown > lcd_height - 18)
                Clear_Screen(); // 清空屏幕
            else
                Draw_Point(Xdown, Ydown, RED); // 画图
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

void communication_task(void *pvParameters)
{
    while (1)
    {
        log_i("communication task is running.");
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
    LCD_Init();        // 初始化LCD FSMC接口和显示驱动
    BRUSH_COLOR = RED; // 设置画笔颜色为红色
    LCD_DisplayString(10, 10, 24, "Illuminati STM32");
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
