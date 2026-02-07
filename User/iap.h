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

// 差分算法枚举
typedef enum
{
    DIFF_ALGO_BSDIFF = 0,   // bsdiff算法
    DIFF_ALGO_XDELTA = 1,   // xdelta算法
    DIFF_ALGO_HDIFF = 2,    // hdiff算法
    DIFF_ALGO_ZDIFF = 3,    // zdelta算法
    DIFF_ALGO_CUSTOM = 255, // 自定义算法
} diff_algorithm_e;

#pragma pack(push) // 保存当前对齐状态
#pragma pack(1)    // 设置为1字节对齐
typedef struct
{
    // 控制标志
    uint8_t running_partition;  // 当前运行的分区 (0 或 1)
    uint8_t ota_in_progress;    // OTA升级进行中标志
    uint8_t rollback_requested; // 请求回滚到之前版本
    uint8_t factory_reset;      // 恢复出厂设置标志
    uint8_t last_boot_success;  // 上次启动是否成功

    // 分区信息
    struct
    {
        img_status_e img_status;   // 状态码
        uint32_t original_size;    // 原始大小
        uint32_t compressed_size;  // 压缩后大小
        uint32_t crc32;            // CRC32校验
        uint16_t version_major;    // 主版本
        uint16_t version_minor;    // 次版本
        uint16_t version_patch;    // 修订版本
        uint32_t timestamp;        // 更新时间
        uint8_t aes_secret_iv[16]; // aes解密密钥
        uint8_t hash_sha256[32];   // SHA-256哈希
    } partition[2];                // 两个分区

    // 诊断
    uint32_t boot_count; // 启动次数
    uint8_t boot_mode;   // 启动模式：0=正常，1=安全模式，2=恢复模式
    uint8_t error_code;  // 最近一次错误代码

    // OTA传输
    uint32_t packet_count;     // app总包数
    uint32_t received_packets; // 已接收包数
    uint32_t total_size;       // 总固件大小
    uint32_t received_bytes;   // 已接收字节数
    uint32_t packet_size;      // 每个包的大小（字节）
    uint8_t encrypt;           // 是否加密
    uint8_t lz;                // 是否压缩
    uint8_t delta_update;      // 升级类型：0=全量升级，1=差分升级
    uint16_t retry_count;      // 重试次数

    // 安全验证
    uint8_t signature_ecdsa[64]; // 数字签名 secp256r1 (NIST P-256) 64字节 128位 最常见，推荐
    uint32_t magic;              // 魔数标识，固定为IAP_MAGIC_NUMBER

    uint32_t stuct_crc32; // 整个结构体的CRC32校验值
} iap_information_t;
#pragma pack(pop) // 恢复之前的对齐状态

#define CRC16_CCITT 0x1021    // 最常用，用于XMODEM、YMODEM
#define CRC32_IEEE 0x04C11DB7 // 最常用，ZIP、PNG、以太网
#define IAP_INFO_SIZE (sizeof(iap_information_t))
#define IAP_RX_LEN 256
extern uint8_t iap_dma_rx_buf[IAP_RX_LEN] __attribute__((aligned(4))); // 禁止编译器优化，对齐4字节;
void iap_init(void);
void iap_rx_event_callback(uint32_t Size);
void JumpToApp(void);
void iap_uart_proceess(void);

#endif /* __IAP_H__ */
