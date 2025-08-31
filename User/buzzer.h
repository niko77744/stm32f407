#ifndef __BUZZER_H__
#define __BUZZER_H__

#include "main.h"

#define BEEP_SHORT_TIME 100
#define BEEP_LONG_TIME 300

typedef enum
{
    beep_short = 0,
    beep_long,
} buzzer_mode_e;

void beep_start(buzzer_mode_e mode);

#endif /* __BUZZER_H__ */
