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
#define SUPPORT_SHELL 1
#define SUPPORT_LOG 1

#if SUPPORT_OS == 1
#define ff_malloc pvPortMalloc
#define ff_free vPortFree
#else
#define ff_malloc malloc
#define ff_free free
#endif

void app_init(void);
void app_run(void);

#endif /* __APP_H__ */
