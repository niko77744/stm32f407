#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include "main.h"

typedef struct
{
    uint64_t hours;
    uint64_t minutes;
    uint64_t seconds;
} sys_time_t;

extern sys_time_t sys_time;

void sw_time_init(void);
void get_sys_time(sys_time_t *time);
void sw_timer_loop(void);
void sys_time_init(void);
void delay_ms(uint16_t nms);
void delay_us(uint32_t nus);

#endif /* __SYS_TIME_H__ */
