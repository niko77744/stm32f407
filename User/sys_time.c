#define LOG_TAG "sys_time"
#include "sys_time.h"
#include "elog.h"

MultiTimer sys_timer;
sys_time_t sys_time;
static const uint8_t fac_us = SYSCLK; // us延时倍乘数

static uint64_t get_platform_tick(void)
{
    return HAL_GetTick(); // 获取系统Tick
}

void get_sys_time(sys_time_t *time)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    uint64_t current_tick = get_platform_tick();
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
    time->hours = sTime.Hours;
    time->minutes = sTime.Minutes;
    time->seconds = sTime.Seconds;
}

static void sys_timer_callback(MultiTimer *timer, void *userData)
{
    get_sys_time(&sys_time);
    // log_i("Time-- %llu:%llu:%llu",
    //       sys_time.hours, sys_time.minutes, sys_time.seconds);
    multiTimerStart(&sys_timer, 1000, sys_timer_callback, NULL);
}

void sys_time_init(void)
{
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    const char *build_date = __DATE__;
    const char *build_time = __TIME__;
    // 解析编译时间
    char month[4];
    int day, year;
    sscanf(build_date, "%s %d %d", month, &day, &year);

    // 解析编译时间
    int hour, minute, second;
    sscanf(build_time, "%d:%d:%d", &hour, &minute, &second);

    // 设置RTC时间
    sTime.Hours = hour;
    sTime.Minutes = minute;
    sTime.Seconds = second;

    sDate.Date = day;
    sDate.Year = year - 2000; // RTC年份从2000开始

    // 月份转换
    const char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    for (int i = 0; i < 12; i++)
    {
        if (strcmp(month, months[i]) == 0)
        {
            sDate.Month = i + 1;
            break;
        }
    }

    HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    // 必须在HAL_RTC_GetTime()之后调用HAL_RTC_GetDate()来解锁高阶日历阴影寄存器中的值，以确保时间和日期值之间的一致性，否则会被上锁。
    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    log_i("Time--%d-%d-%d %d:%d:%d\r\n",
          sDate.Year, sDate.Month, sDate.Date,
          sTime.Hours, sTime.Minutes, sTime.Seconds);
    multiTimerStart(&sys_timer, 1000, sys_timer_callback, NULL);
}

void sw_time_init(void)
{
    multiTimerInstall(get_platform_tick);
}

void sw_timer_loop(void)
{
    multiTimerYield();
}

void delay_us(uint32_t nus)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD; // LOAD的值
    ticks = nus * fac_us;            // 需要的节拍数
    told = SysTick->VAL;             // 刚进入时的计数器值
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow; // 这里注意一下SYSTICK是一个递减的计数器就可以了.
            else
                tcnt += reload - tnow + told;
            told = tnow;
            if (tcnt >= ticks)
                break; // 时间超过/等于要延迟的时间,则退出.
        }
    }
}

/**
 * @brief     延时nms
 * @param     nms: 要延时的ms数 (0< nms <= 65535)
 * @retval    无
 */
void delay_ms(uint16_t nms)
{
    uint32_t i;

    for (i = 0; i < nms; i++)
    {
        delay_us(1000);
    }
}
