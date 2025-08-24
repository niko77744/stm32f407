#ifndef __SD_CARD_H__
#define __SD_CARD_H__

#include "main.h"

#define SD_TIMEOUT ((uint32_t)100000000) // 超时时间
#define SD_TRANSFER_OK ((uint8_t)0x00)
#define SD_TRANSFER_BUSY ((uint8_t)0x01)
// SD卡块大小
#define SD_BLOCKSIZE 512
// 获取卡的状态
uint8_t SD_GetCardInfo(HAL_SD_CardInfoTypeDef *cardinfo);
uint8_t SD_GetCardState(void);
uint8_t SD_ReadSDisk(uint8_t *buf, uint32_t sector, uint32_t cnt);
uint8_t SD_WriteSDisk(uint8_t *buf, uint32_t sector, uint32_t cnt);
uint8_t SD_ReadBlocks_DMA(uint32_t *buf, uint64_t sector, uint32_t blocksize, uint32_t cnt);
uint8_t SD_WriteBlocks_DMA(uint32_t *buf, uint64_t sector, uint32_t blocksize, uint32_t cnt);
void log_sd_card_info(void);

#endif /* __SD_CARD_H__ */
