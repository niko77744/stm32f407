#ifndef __IAP_H__
#define __IAP_H__

#include "main.h"

typedef enum
{
    iap_source_none = 0,
    iap_from_uart,
    iap_from_ble,
    iap_from_wifi,
    iap_from_i2c,
    iap_from_swd,
    iap_from_can,
    iap_from_eth,
    iap_from_norflash,
    iap_from_usb,
    iap_from_sdcard,
    iap_from_max
} iap_source_t;

typedef enum
{
    iap_err_none = 0,
    iap_err_source,
    iap_err_size,
    iap_err_timeout,
    iap_err_data,
    iap_err_program,
    iap_err_verify,
    iap_err_max
} iap_err_e;

typedef enum
{
    iap_state_idle = 0,
    iap_state_start,
    iap_state_receiving,
    iap_state_programming,
    iap_state_verify,
    iap_state_finish,
    iap_state_error,
    iap_state_max
} iap_state_e;

typedef struct
{
    iap_source_t source;   // 升级来源
    iap_err_e err_code;    // 错误代码
    iap_state_e state;     // 当前状态
    uint32_t program_size; // 已写入大小
    uint32_t start_addr;   // 固件起始地址
    uint32_t timeout;      // 超时时间
} iap_t;

#endif /* __IAP_H__ */
