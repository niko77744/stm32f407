#define LOG_TAG "msg_queue"
#include "msg_queue.h"
#include "lwevt.h"
#include "elog.h"

lwevt_t evt_local;

// ����Ҫ��Ч����������Դ��Ӧ�Դ����첽����ʱ�������¼������ܹ�����������ϵͳ�����ܺͿɿ��ԡ�

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

    // ��ʼ���¼�ϵͳ
    lwevt_init();

    // ע�������û��¼�������
    lwevt_register(prv_evt_fn_1);
    lwevt_register(prv_evt_fn_2);

    /*
     * Send event using global handle
     * Thread safety has to be ensured in multi-threading environments
     */
    evt = lwevt_get_handle(); // ��ȡĬ���¼����
    evt->msg.ext1.par1 = 1;
    evt->msg.ext1.par2 = 2;
    lwevt_dispatch(LWEVT_TYPE_MY_EXT_1); // �ַ��¼������м����������յ�

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
    lwevt_dispatch_ex(&evt_local, LWEVT_TYPE_MY_EXT_2); // �ñ��ؾ���ַ��¼�
}
