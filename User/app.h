#ifndef __APP_H__
#define __APP_H__

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

#define SUPPORT_OS 1
#define SUPPORT_LVGL 0

void app_init(void);
void app_os_start(void);

#endif /* __APP_H__ */
