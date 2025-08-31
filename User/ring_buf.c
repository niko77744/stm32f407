#define LOG_TAG "ring_buf"
#include "ring_buf.h"
#include "elog.h"
#include "lwrb.h"

/*
https://blog.csdn.net/qq_36075612/article/details/116144349
https://docs.majerle.eu/projects/lwrb/en/latest/api-reference/lwrb.html
*/

static lwrb_t buff;
static uint8_t buff_data[8 + 1]; // ����������������8�ֽڡ����ζ��������1���ֽڡ�

void ring_buf_init(void)
{
    // ��2���ֽ�
    uint8_t data[2];
    size_t len;

    lwrb_init(&buff, buff_data, sizeof(buff_data)); // ��ʼ��
    lwrb_is_ready(&buff);                           // ����Ƿ��ʼ���ɹ�
    size_t count_w = lwrb_get_free(&buff);          // 8���ֽڿ�д ��ȡ������������д�����Ŀ��ô�С��
    size_t count_r = lwrb_get_full(&buff);          // 0���ֽڿɶ� ��ȡ�������е�ǰ���õ��ֽ�����
    lwrb_write(&buff, "0123", 4);                   // д��4���ֽ� ����������еĿ����ڴ���٣����Ƶ��ڴ���١��û������麯���ķ���ֵ�������������д�볤�Ƚ��бȽϣ���ȷ���Ƿ���д����������

    /* Read until len == 0, when buffer is empty */
    while ((len = lwrb_read(&buff, data, sizeof(data))) > 0)
    {
        log_i("Successfully read %d bytes", (int)len);
        log_i("count_r %d bytes", (int)lwrb_get_full(&buff));
    }

    // �ȿ������ӵ�������lwrb_peek��lwrb_skip�� lwrb_peek�ӻ�������ȡ�������Ķ�ȡָ�루��������
    // while ((len = lwrb_peek(&buff, 0, data, sizeof(data))) > 0)
    // {
    //     log_i("Successfully read %d bytes", (int)len);
    //     log_i("count_r %d bytes", (int)lwrb_get_full(&buff));
    //     lwrb_skip(&buff, sizeof(data)); // peek��skip���൱�ڶ��е�peek��pop
    // }
}
