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

void sw_time_init(void)
{
    multiTimerInstall(get_platform_tick);
}

void sw_timer_loop(void)
{
    multiTimerYield();
}
