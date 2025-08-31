#define LOG_TAG "msg_queue"
#include "msg_queue.h"
#include "lwevt.h"
#include "elog.h"

lwevt_t evt_local;

// 当需要高效利用有限资源或应对大量异步操作时，采用事件驱动架构能显著提升系统的性能和可靠性。

static void prv_evt_fn_1(lwevt_t *e)
{
    switch ((unsigned)e->type)
    {
    case LWEVT_TYPE_MY_EXT_1:
    {
        log_i("Event fn 1, LWEVT_TYPE_MY_EXT_1 - with data: a: %d, b: %d",
              (int)e->msg.ext1.par1, (int)e->msg.ext1.par2);
        break;
    }
    case LWEVT_TYPE_MY_BASIC_1:
    {
        log_i("Basic event type - no data");
        break;
    }
    default:
        break;
    }
}

static void prv_evt_fn_2(lwevt_t *e)
{
    switch ((unsigned)e->type)
    {
    case LWEVT_TYPE_MY_EXT_2:
    {
        log_i("Event fn 2, LWEVT_TYPE_MY_EXT_2 - with data: a: %d, b: %d",
              (int)e->msg.ext2.par3, (int)e->msg.ext2.par4);
        break;
    }
    default:
        break;
    }
}

void message_queue_init(void)
{
    lwevt_t *evt;

    // 初始化事件系统
    lwevt_init();

    // 注册两个用户事件监听器
    lwevt_register(prv_evt_fn_1);
    lwevt_register(prv_evt_fn_2);

    /*
     * Send event using global handle
     * Thread safety has to be ensured in multi-threading environments
     */
    evt = lwevt_get_handle(); // 获取默认事件句柄
    evt->msg.ext1.par1 = 1;
    evt->msg.ext1.par2 = 2;
    lwevt_dispatch(LWEVT_TYPE_MY_EXT_1); // 分发事件，所有监听器都会收到

    /*
     * Get event handle, set event data and dispatch event
     *
     * Send basic event - without any data
     */
    evt = lwevt_get_handle();
    lwevt_dispatch(LWEVT_TYPE_MY_BASIC_1);

    /*
     * Send event using local handle
     * No need to ensure thread safety
     */
    evt_local.msg.ext2.par3 = 3;
    evt_local.msg.ext2.par4 = 4;
    lwevt_dispatch_ex(&evt_local, LWEVT_TYPE_MY_EXT_2); // 用本地句柄分发事件
}
