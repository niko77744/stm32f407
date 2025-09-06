#define LOG_TAG "sys_time"
#include "sys_time.h"
#include "elog.h"

MultiTimer sys_timer;
sys_time_t sys_time;
static const uint8_t fac_us = SYSCLK; // us��ʱ������

static uint64_t get_platform_tick(void)
{
    return HAL_GetTick(); // ��ȡϵͳTick
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
    uint32_t reload = SysTick->LOAD; // LOAD��ֵ
    ticks = nus * fac_us;            // ��Ҫ�Ľ�����
    told = SysTick->VAL;             // �ս���ʱ�ļ�����ֵ
    while (1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
                tcnt += told - tnow; // ����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
            else
                tcnt += reload - tnow + told;
            told = tnow;
            if (tcnt >= ticks)
                break; // ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
        }
    }
}

/**
 * @brief     ��ʱnms
 * @param     nms: Ҫ��ʱ��ms�� (0< nms <= 65535)
 * @retval    ��
 */
void delay_ms(uint16_t nms)
{
    uint32_t i;

    for (i = 0; i < nms; i++)
    {
        delay_us(1000);
    }
}
