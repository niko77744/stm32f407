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

extern const struct fal_partition *fal_little_fs_partition;

#endif /* __APP_H__ */
