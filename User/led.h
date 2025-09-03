#ifndef __LED_H__
#define __LED_H__

#include "main.h"

typedef enum
{
    LED0 = 0,
    LED1,
    LED2,
} LED_TypeDef;

void led_on(LED_TypeDef led);
void led_off(LED_TypeDef led);
void led_toggle(LED_TypeDef led);

#endif /* __LED_H__ */
