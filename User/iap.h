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

typedef enum
{
    IMG_VALID = 0,      // 没有限制，可以选取。
    IMG_UNDEFINED,      // 没有限制，可以选取。
    IMG_INVALID,        // 不会选取。
    IMG_ABORTED,        // 不会选取。
    IMG_NEW,            // 则仅会选取一次。在引导加载程序中，状态立即变为 IMG_PENDING_VERIFY
    IMG_PENDING_VERIFY, // 不会选取，状态变为 IMG_ABORTED
} img_status_e;

#pragma pack(push) // 保存当前对齐状态
#pragma pack(1)    // 设置为1字节对齐
typedef struct
{
    uint8_t running_partition;  // 当前运行的分区 (1 或 2)
    uint8_t ota_in_progress;    // OTA升级进行中标志
    uint8_t boot_attempts;      // 启动尝试次数（用于防止启动循环）
    uint8_t rollback_requested; // 请求回滚到之前版本

    uint32_t app1_addr; // 应用程序分区1的起始地址
    uint32_t app1_size; // 分区1的固件大小
    uint32_t app2_addr; // 应用程序分区2的起始地址
    uint32_t app2_size; // 分区2的固件大小

    // 分区状态管理
    img_status_e app1_status; // 分区1的状态
    img_status_e app2_status; // 分区2的状态

    uint32_t crc32;  // 整个结构体的CRC32校验值
} iap_information_t; // __attribute__((packed))
#pragma pack(pop)    // 恢复之前的对齐状态

#define FLASH_APP1_ADDR 0x08010000 /* 第一个应用程序起始地址(存放在内部FLASH)              \
                                    * 保留 0x08000000~0x0800FFFF 的空间为 Bootloader 使用(共64KB) \
                                    */
void iap_init(iap_source_t source);
void log_rx_event_callback(uint16_t Size);
void iap_write_appbin(uint32_t appxaddr, uint8_t *appbuf, uint32_t appsize);
void iap_load_app(uint32_t appxaddr);
void iap_process(void);
void iap_uart_proceess(void);
#endif /* __IAP_H__ */
