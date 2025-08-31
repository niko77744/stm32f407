#define LOG_TAG "ring_buf"
#include "ring_buf.h"
#include "elog.h"
#include "lwrb.h"

/*
https://blog.csdn.net/qq_36075612/article/details/116144349
https://docs.majerle.eu/projects/lwrb/en/latest/api-reference/lwrb.html
*/

static lwrb_t buff;
static uint8_t buff_data[8 + 1]; // 数组容器。可容纳8字节。环形队列需多留1个字节。

void ring_buf_init(void)
{
    // 读2个字节
    uint8_t data[2];
    size_t len;

    lwrb_init(&buff, buff_data, sizeof(buff_data)); // 初始化
    lwrb_is_ready(&buff);                           // 检测是否初始化成功
    size_t count_w = lwrb_get_free(&buff);          // 8个字节可写 获取缓冲区中用于写入作的可用大小。
    size_t count_r = lwrb_get_full(&buff);          // 0个字节可读 获取缓冲区中当前可用的字节数。
    lwrb_write(&buff, "0123", 4);                   // 写入4个字节 如果缓冲区中的可用内存较少，则复制的内存较少。用户必须检查函数的返回值并将其与请求的写入长度进行比较，以确定是否已写入所有内容

    /* Read until len == 0, when buffer is empty */
    while ((len = lwrb_read(&buff, data, sizeof(data))) > 0)
    {
        log_i("Successfully read %d bytes", (int)len);
        log_i("count_r %d bytes", (int)lwrb_get_full(&buff));
    }

    // 先窥读后扔掉，即先lwrb_peek后lwrb_skip。 lwrb_peek从缓冲区读取而不更改读取指针（仅速览）
    // while ((len = lwrb_peek(&buff, 0, data, sizeof(data))) > 0)
    // {
    //     log_i("Successfully read %d bytes", (int)len);
    //     log_i("count_r %d bytes", (int)lwrb_get_full(&buff));
    //     lwrb_skip(&buff, sizeof(data)); // peek后skip，相当于队列的peek后pop
    // }
}
