#define LOG_TAG "rf433"
#include "rf433.h"
#include "tim.h"

void rf433_init(void)
{
    __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC1 | TIM_IT_CC2);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
}

// 1代表100us
#define RF433_MS(n) ((uint32_t)((n) * 10.0f))
// 时间检查辅助宏
#define TIME_IN_RANGE(t, min, max) ((t) >= (min) && (t) <= (max))
#define NEC_DATA_BITS_TOTAL (32) // NEC协议总位数
#define NEC_BITS_PER_BYTE (8)    // 每字节位数
// 接收状态
typedef enum
{
    STATE_WAIT_LEADER = 0,
    STATE_RECEIVE_DATA,
    STATE_COMPLETE
} nec_recv_state_e;

// 接收数据结构
typedef struct
{
    uint8_t addr;
    uint8_t cmd;
} nec_data_t;

// 全局变量
struct
{
    nec_recv_state_e state;
    uint8_t bit_cnt;
    uint32_t raw_data;
    uint32_t low_time;  // 低电平时间
    uint32_t high_time; // 高电平时间
    nec_data_t result;
} nec_recv = {0};

// NEC数据校验
static uint8_t nec_check(uint8_t addr, uint8_t addr_inv, uint8_t cmd, uint8_t cmd_inv)
{
    return ((uint8_t)~addr == addr_inv) && ((uint8_t)~cmd == cmd_inv);
}

// 中断回调函数 - 简洁版
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance != TIM1)
        return;

    // 上升沿捕获（低电平结束）
    if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
    {
        nec_recv.low_time = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
        __HAL_TIM_SET_COUNTER(htim, 0);
    }
    // 下降沿捕获（高电平结束）
    else if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2)
    {
        nec_recv.high_time = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);
        __HAL_TIM_SET_COUNTER(htim, 0);

        switch (nec_recv.state)
        {
        case STATE_WAIT_LEADER:
            // 检查引导码
            if (TIME_IN_RANGE(nec_recv.low_time, RF433_MS(8.0f), RF433_MS(10.0f)) &&
                TIME_IN_RANGE(nec_recv.high_time, RF433_MS(4.0f), RF433_MS(5.0f)))
            {
                nec_recv.state = STATE_RECEIVE_DATA;
                nec_recv.bit_cnt = 0;
                nec_recv.raw_data = 0;
            }
            break;

        case STATE_RECEIVE_DATA:
            if (!TIME_IN_RANGE(nec_recv.low_time, RF433_MS(0.4f), RF433_MS(0.9f)) ||
                !TIME_IN_RANGE(nec_recv.high_time, RF433_MS(0.4f), RF433_MS(1.9f)))
            {
                nec_recv.state = STATE_COMPLETE;
                break;
            }

            // 解析数据位
            nec_recv.raw_data <<= 1;
            if (nec_recv.high_time >= RF433_MS(1.0f))
            {
                nec_recv.raw_data |= 1; // 逻辑1
            }

            nec_recv.bit_cnt++;
            // 检查是否接收完32位
            if (nec_recv.bit_cnt >= NEC_DATA_BITS_TOTAL)
            {
                nec_recv.state = STATE_COMPLETE;
            }
            break;
        }

        // 接收完成，解析数据
        if (nec_recv.state == STATE_COMPLETE)
        {
            // 按接收顺序：地址->地址反码->命令->命令反码
            uint8_t *p = (uint8_t *)&nec_recv.raw_data;
            uint8_t addr = p[3];
            uint8_t addr_inv = p[2];
            uint8_t cmd = p[1];
            uint8_t cmd_inv = p[0];

            uint8_t result = nec_check(addr, addr_inv, cmd, cmd_inv);
            if (result)
            {
                nec_recv.result.addr = addr;
                nec_recv.result.cmd = cmd;
                beep_start(beep_short);
            }

            // 重置状态，等待下一帧
            nec_recv.state = STATE_WAIT_LEADER;
        }
    }
}
