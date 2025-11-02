#ifndef __SD_CARD_H__
#define __SD_CARD_H__

#include "main.h"

#define SD_TRANSFER_OK ((uint8_t)0x00)
#define SD_TRANSFER_BUSY ((uint8_t)0x01)
// SD¿¨¿é´óÐ¡
#define SD_BLOCKSIZE 512

void log_sd_card_info(void);
void sd_fatfs_init(void);
void sd_fatfs_self_inspection(void);

#endif /* __SD_CARD_H__ */
