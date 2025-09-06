/*
 * FreeRTOS V202212.00
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* Ensure stdint is only used by the compiler, and not the assembler. */

#include <stdint.h>
extern uint32_t SystemCoreClock;

#define configUSE_PREEMPTION 1                         /* 1: ��ռʽ������, 0: Э��ʽ������, ��Ĭ���趨�� */
#define configUSE_IDLE_HOOK 1                          /* 1: ʹ�ܿ��������Ӻ���, ��Ĭ���趨��  */
#define configUSE_TICK_HOOK 1                          /* 1: ʹ��ϵͳʱ�ӽ����жϹ��Ӻ���, ��Ĭ���趨�� */
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0           /* 1: ʹ�ܶ�ʱ�����������״�ִ��ǰ�Ĺ��Ӻ���, Ĭ��: 0 */
#define configCPU_CLOCK_HZ (SystemCoreClock)           /* ����CPU��Ƶ, ��λ: Hz, ��Ĭ���趨�� */
#define configTICK_RATE_HZ ((TickType_t)1000)          /* ����ϵͳʱ�ӽ���Ƶ��, ��λ: Hz, ��Ĭ���趨�� */
#define configMAX_PRIORITIES (5)                       /* ����������ȼ���, ������ȼ�=configMAX_PRIORITIES-1, ��Ĭ���趨�� */
#define configMINIMAL_STACK_SIZE ((unsigned short)128) /* ������������ջ�ռ��С, ��λ: Word, ��Ĭ���趨�� */
#define configTOTAL_HEAP_SIZE ((size_t)(64 * 1024))    /* FreeRTOS���п��õ�RAM����, ��λ: Byte, ��Ĭ���趨�� */
#define configMAX_TASK_NAME_LEN (10)                   /* ��������������ַ���, Ĭ��: 16 */
#define configUSE_TRACE_FACILITY 1                     /* 1: ʹ�ܿ��ӻ����ٵ���, Ĭ��: 0 */
#define configUSE_16_BIT_TICKS 0                       /* 1: ����ϵͳʱ�ӽ��ļ���������������Ϊ16λ�޷�����, ��Ĭ���趨�� */
#define configIDLE_SHOULD_YIELD 0                      /* 1: ʹ������ռʽ������,ͬ���ȼ�����������ռ��������, Ĭ��: 1 */
#define configUSE_MUTEXES 1                            /* 1: ʹ�ܻ����ź���, Ĭ��: 0 */
#define configQUEUE_REGISTRY_SIZE 8                    /* �������ע����ź�������Ϣ���еĸ���, Ĭ��: 0 */
#define configCHECK_FOR_STACK_OVERFLOW 2               /* 1: ʹ��ջ�����ⷽ��1, 2: ʹ��ջ�����ⷽ��2, Ĭ��: 0 */
#define configUSE_RECURSIVE_MUTEXES 1                  /* 1: ʹ�ܵݹ黥���ź���, Ĭ��: 0 */
#define configUSE_MALLOC_FAILED_HOOK 1                 /* 1: ʹ�ܶ�̬�ڴ�����ʧ�ܹ��Ӻ���, Ĭ��: 0 */
#define configUSE_COUNTING_SEMAPHORES 1                /* 1: ʹ�ܼ����ź���, Ĭ��: 0 */
#define configGENERATE_RUN_TIME_STATS 0                /* 1: ʹ����������ʱ��ͳ�ƹ���, Ĭ��: 0 */
#define configUSE_QUEUE_SETS 1                         /* 1: ʹ�ܶ��м�, Ĭ��: 0 */
#define configUSE_TASK_NOTIFICATIONS 1                 /* 1: ʹ�������ֱ�ӵ���Ϣ����,�����ź������¼���־�����Ϣ����, Ĭ��: 1 */
#define configTASK_NOTIFICATION_ARRAY_ENTRIES 1        /* ��������֪ͨ����Ĵ�С, Ĭ��: 1 */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1      /* 1: ʹ��Ӳ��������һ��Ҫ���е�����, 0: ʹ������㷨������һ��Ҫ���е�����, Ĭ��: 0 */
#define configUSE_TICKLESS_IDLE 0                      /* 1: ʹ��tickless�͹���ģʽ, Ĭ��: 0 */
#define configUSE_TIME_SLICING 1                       /* 1: ʹ��ʱ��Ƭ����, Ĭ��: 1 */
#define configUSE_NEWLIB_REENTRANT 0                   /* 1: ���񴴽�ʱ����Newlib������ṹ��, Ĭ��: 0 */
#define configENABLE_BACKWARD_COMPATIBILITY 0          /* 1: ʹ�ܼ����ϰ汾, Ĭ��: 1 */
#define configUSE_APPLICATION_TASK_TAG 0               /* */

/* �ڴ������ض��� */
#define configSUPPORT_STATIC_ALLOCATION 0  /* 1: ֧�־�̬�����ڴ�, Ĭ��: 0 */
#define configSUPPORT_DYNAMIC_ALLOCATION 1 /* 1: ֧�ֶ�̬�����ڴ�, Ĭ��: 1 */
#define configAPPLICATION_ALLOCATED_HEAP 1 /* 1: �û��ֶ�����FreeRTOS�ڴ��(ucHeap), Ĭ��: 0 */

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 0             /* 1: ����Э��, Ĭ��: 0 */
#define configMAX_CO_ROUTINE_PRIORITIES (2) /* ����Э�̵�������ȼ�, ������ȼ�=configMAX_CO_ROUTINE_PRIORITIES-1, ��Ĭ��configUSE_CO_ROUTINESΪ1ʱ�趨�� */

/* Software timer definitions. */
#define configUSE_TIMERS 1                                          /* 1: ʹ�������ʱ��, Ĭ��: 0 */
#define configTIMER_TASK_PRIORITY (2)                               /* ���������ʱ����������ȼ�, ��Ĭ��configUSE_TIMERSΪ1ʱ�趨�� */
#define configTIMER_QUEUE_LENGTH 10                                 /* ���������ʱ��������еĳ���, ��Ĭ��configUSE_TIMERSΪ1ʱ�趨�� */
#define configTIMER_TASK_STACK_DEPTH (configMINIMAL_STACK_SIZE * 2) /* ���������ʱ�������ջ�ռ��С, ��Ĭ��configUSE_TIMERSΪ1ʱ�趨�� */

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */
#define INCLUDE_vTaskPrioritySet 1            /* �����������ȼ� */
#define INCLUDE_uxTaskPriorityGet 1           /* ��ȡ�������ȼ� */
#define INCLUDE_vTaskDelete 1                 /* ɾ������ */
#define INCLUDE_xResumeFromISR 1              /* �ָ����ж��й�������� */
#define INCLUDE_vTaskSuspend 1                /* �������� */
#define INCLUDE_vTaskDelayUntil 1             /* ���������ʱ */
#define INCLUDE_vTaskDelay 1                  /* ������ʱ */
#define INCLUDE_xTaskGetSchedulerState 1      /* ��ȡ���������״̬ */
#define INCLUDE_xTaskGetCurrentTaskHandle 1   /* ��ȡ��ǰ����������� */
#define INCLUDE_uxTaskGetStackHighWaterMark 1 /* ��ȡ�����ջ��ʷʣ����Сֵ */
#define INCLUDE_xTaskGetIdleTaskHandle 1      /* ��ȡ��������������� */
#define INCLUDE_eTaskGetState 1               /* ��ȡ����״̬ */
#define INCLUDE_xEventGroupSetBitFromISR 1    /* ���ж��������¼���־λ */
#define INCLUDE_xTimerPendFunctionCall 1      /* ��������ִ�йҵ���ʱ���������� */
#define INCLUDE_xTaskAbortDelay 1             /* �ж�������ʱ */
#define INCLUDE_xTaskGetHandle 1              /* ͨ����������ȡ������ */
#define INCLUDE_xTaskResumeFromISR 1          /* �ָ����ж��й�������� */
#define INCLUDE_vTaskCleanUpResources 1

/* Cortex-M specific definitions. */
/* �ж�Ƕ����Ϊ���� */
#ifdef __NVIC_PRIO_BITS
/* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
#define configPRIO_BITS __NVIC_PRIO_BITS
#else
#define configPRIO_BITS 4 /* 15 priority levels */
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 0xf /* �ж�������ȼ� */

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5 /* FreeRTOS�ɹ��������ж����ȼ� */

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
// #define configASSERT(x) \
//     if ((x) == 0)       \
//     vAssertCalled(__FILE__, __LINE__)

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names. */
#define vPortSVCHandler SVC_Handler
#define xPortPendSVHandler PendSV_Handler
// #define xPortSysTickHandler SysTick_Handler

/* FreeRTOS MPU ���ⶨ�� */
// #define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0
// #define configTOTAL_MPU_REGIONS                                8
// #define configTEX_S_C_B_FLASH                                  0x07UL
// #define configTEX_S_C_B_SRAM                                   0x07UL
// #define configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY            1
// #define configALLOW_UNPRIVILEGED_CRITICAL_SECTIONS             1

/* ARMv8-M ��ȫ��˿���ض��塣 */
// #define secureconfigMAX_SECURE_CONTEXTS         5

#endif /* FREERTOS_CONFIG_H */
