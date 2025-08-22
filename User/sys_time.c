#define LOG_TAG "sys_time"
#include "sys_time.h"
#include "elog.h"

MultiTimer log_timer;
sys_time_t sys_time;

static uint64_t get_platform_tick(void)
{
    return HAL_GetTick(); // 获取系统Tick
}

void get_sys_time(sys_time_t *time)
{
    uint64_t current_tick = get_platform_tick();
    uint64_t total_seconds = current_tick / 1000;
    time->hours = total_seconds / 3600;
    time->minutes = (total_seconds % 3600) / 60;
    time->seconds = total_seconds % 60;
}

static void log_timer_callback(MultiTimer *timer, void *userData)
{
    get_sys_time(&sys_time);
    log_i("Log timer callback triggered, Timer fired at %llu hour: %llu minute: %llu second", sys_time.hours, sys_time.minutes, sys_time.seconds);
    multiTimerStart(&log_timer, 5000, log_timer_callback, NULL);
}

void start_log_timer(void)
{
    multiTimerInstall(get_platform_tick);
    multiTimerStart(&log_timer, 5000, log_timer_callback, NULL);
}

void timer_loop(void)
{
    multiTimerYield();
}
