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
    uint64_t current_tick = get_platform_tick();
    uint64_t total_seconds = current_tick / 1000;
    time->hours = total_seconds / 3600;
    time->minutes = (total_seconds % 3600) / 60;
    time->seconds = total_seconds % 60;
}

static void sys_timer_callback(MultiTimer *timer, void *userData)
{
    get_sys_time(&sys_time);
    multiTimerStart(&sys_timer, 500, sys_timer_callback, NULL);
}

void sys_time_init(void)
{
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
