#define LOG_TAG "msg_queue"
#include "msg_queue.h"
#include "lwevt.h"
#include "elog.h"

void message_queue_init(void)
{
    // 初始化事件系统
    lwevt_init();
}
