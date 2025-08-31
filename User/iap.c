#define LOG_TAG "iap"
#include "iap.h"

#pragma diag_suppress 177 // ���Ʊ��ļ��е� 177 ���� -- ����δʹ�õĺ�������

#define IAP_BUF_SIZE 1024
iap_t iap_handle;
uint8_t iap_buf[IAP_BUF_SIZE] = {0};

void iap_init(iap_source_t source, uint32_t start_addr, uint32_t timeout)
{
    iap_handle.source = source;
    iap_handle.err_code = iap_err_none;
    iap_handle.state = iap_state_idle;
    iap_handle.program_size = 0;
    iap_handle.start_addr = start_addr;
    iap_handle.timeout = timeout;
}

iap_err_e iap_recv_program_flash(uint8_t *data, uint32_t size)
{
    return iap_err_none;
}

/*
*********************************************************************************************************
*	�� �� ��: JumpToBootloader
*	����˵��: ��ת��ϵͳBootLoader
*	��    ��: ��
*	�� �� ֵ: ��
*********************************************************************************************************
*/

static void JumpToSysBootloader(void)
{
    uint32_t i = 0;
    void (*SysMemBootJump)(void);        /* ����һ������ָ�� */
    __IO uint32_t BootAddr = 0x1FFF0000; /* STM32F4��ϵͳBootLoader��ַ */

    /* �ر�ȫ���ж� __disable_irq(); */
    __set_PRIMASK(1);

    /* �رյδ�ʱ������λ��Ĭ��ֵ */
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;

    /* ��������ʱ�ӵ�Ĭ��״̬��ʹ��HSIʱ�� */
    HAL_RCC_DeInit();

    /* �ر������жϣ���������жϹ����־ */
    for (i = 0; i < 8; i++)
    {
        NVIC->ICER[i] = 0xFFFFFFFF;
        NVIC->ICPR[i] = 0xFFFFFFFF;
    }

    /* ʹ��ȫ���ж� __enable_irq */
    __set_PRIMASK(0);

    /* ��ת��ϵͳBootLoader���׵�ַ��MSP����ַ+4�Ǹ�λ�жϷ�������ַ */
    SysMemBootJump = (void (*)(void))(*((uint32_t *)(BootAddr + 4)));

    /* ��������ջָ�� */
    __set_MSP(*(uint32_t *)BootAddr);

    /* ��RTOS���̣�����������Ҫ������Ϊ��Ȩ��ģʽ��ʹ��MSPָ�� */
    __set_CONTROL(0);

    /* ��ת��ϵͳBootLoader */
    SysMemBootJump();

    /* ��ת�ɹ��Ļ�������ִ�е�����û�������������Ӵ��� */
    while (1)
    {
    }
}
