#define LOG_TAG "sd_card"
#include "sd_card.h"
#include "elog.h"

// 得到卡信息
// cardinfo:卡信息存储区
// 返回值:错误状态
uint8_t SD_GetCardInfo(HAL_SD_CardInfoTypeDef *cardinfo)
{
    uint8_t sta;
    sta = HAL_SD_GetCardInfo(&hsd, cardinfo);
    return sta;
}

// 获取卡的状态
uint8_t SD_GetCardState(void)
{
    if (HAL_SD_GetCardState(&hsd) == HAL_SD_CARD_TRANSFER)
    {
        return SD_TRANSFER_OK;
    }
    else
    {
        return SD_TRANSFER_BUSY;
    }
}

// SD卡里面他就是以扇区为大小来进行读取的。也就是SD卡里面每个地址是512个字节。他的最小读写单元就是512个字节，也就是一个扇区。
uint8_t SD_ReadBlocks_DMA(uint32_t *buf, uint64_t sector, uint32_t blocksize, uint32_t cnt)
{
    uint8_t err = 0;
    uint32_t timeout = SD_TIMEOUT;
    err = HAL_SD_ReadBlocks_DMA(&hsd, (uint8_t *)buf, sector, cnt); // 通过DMA读取SD卡n个扇区

    if (err == 0)
    {
        // 等待SD卡读完
        while (SD_GetCardState() != SD_TRANSFER_OK)
        {
            if (timeout-- == 0)
            {
                err = SD_TRANSFER_BUSY;
            }
        }
    }
    return err;
}

uint8_t SD_WriteBlocks_DMA(uint32_t *buf, uint64_t sector, uint32_t blocksize, uint32_t cnt)
{
    uint8_t err = 0;
    uint32_t timeout = SD_TIMEOUT;
    err = HAL_SD_WriteBlocks_DMA(&hsd, (uint8_t *)buf, sector, cnt); // 通过DMA写SD卡n个扇区

    if (err == 0)
    {
        // 等待SD卡写完
        while (SD_GetCardState() != SD_TRANSFER_OK)
        {
            if (timeout-- == 0)
            {
                err = SD_TRANSFER_BUSY;
            }
        }
    }
    return err;
}

uint8_t SD_ReadSDisk(uint8_t *buf, uint32_t sector, uint32_t cnt)
{
    uint8_t sta = HAL_OK;
    long long lsector = sector;

    sta = SD_ReadBlocks_DMA((uint32_t *)buf, lsector, 512, cnt);

    return sta;
}

uint8_t SD_WriteSDisk(uint8_t *buf, uint32_t sector, uint32_t cnt)
{
    uint8_t sta = HAL_OK;
    long long lsector = sector;

    sta = SD_WriteBlocks_DMA((uint32_t *)buf, lsector, 512, cnt); // 多个sector的写操作

    return sta;
}

uint8_t SD_ReadSDisk_NoDma(uint8_t *buf, uint32_t sector, uint32_t cnt)
{
    uint8_t sta = HAL_OK;
    uint32_t timeout = SD_TIMEOUT;
    long long lsector = sector;
    __disable_irq();                                                         // 关闭总中断
    sta = HAL_SD_ReadBlocks(&hsd, (uint8_t *)buf, lsector, cnt, SD_TIMEOUT); // 多个sector的读操作

    // 等待SD卡读完
    while (SD_GetCardState() != SD_TRANSFER_OK)
    {
        if (timeout-- == 0)
        {
            sta = SD_TRANSFER_BUSY;
        }
    }
    __enable_irq(); // 开启总中断
    return sta;
}

uint8_t SD_WriteSDisk_NoDma(uint8_t *buf, uint32_t sector, uint32_t cnt)
{
    uint8_t sta = HAL_OK;
    uint32_t timeout = SD_TIMEOUT;
    long long lsector = sector;
    __disable_irq();                                                          // 关闭总中断(POLLING模式,严禁中断打断SDIO读写操作!!!)
    sta = HAL_SD_WriteBlocks(&hsd, (uint8_t *)buf, lsector, cnt, SD_TIMEOUT); // 多个sector的写操作

    // 等待SD卡写完
    while (SD_GetCardState() != SD_TRANSFER_OK)
    {
        if (timeout-- == 0)
        {
            sta = SD_TRANSFER_BUSY;
        }
    }
    __enable_irq(); // 开启总中断
    return sta;
}

void log_sd_card_info(void)
{
    uint64_t CardCap; // SD卡容量
    HAL_SD_CardCIDTypeDef SDCard_CID;
    HAL_SD_CardInfoTypeDef SDCardInfo;

    HAL_SD_GetCardCID(&hsd, &SDCard_CID);                                               // 获取CID
    HAL_SD_GetCardInfo(&hsd, &SDCardInfo);                                              // 获取SD卡信息
    CardCap = (uint64_t)(SDCardInfo.LogBlockNbr) * (uint64_t)(SDCardInfo.LogBlockSize); // 计算SD卡容量
    switch (SDCardInfo.CardType)
    {
    case CARD_SDSC:
    {
        if (SDCardInfo.CardVersion == CARD_V1_X)
            log_i("Card Type:SDSC V1");
        else if (SDCardInfo.CardVersion == CARD_V2_X)
            log_i("Card Type:SDSC V2");
    }
    break;
    case CARD_SDHC_SDXC:
        log_i("Card Type:SDHC");
        break;
    default:
        break;
    }

    log_i("Card ManufacturerID: %d", SDCard_CID.ManufacturerID);   // 制造商ID
    log_i("CardVersion: %d ", (uint32_t)(SDCardInfo.CardVersion)); // 卡版本号
    log_i("Class: %d ", (uint32_t)(SDCardInfo.Class));
    log_i("Card RCA(RelCardAdd):%d ", SDCardInfo.RelCardAdd);        // 卡相对地址
    log_i("Card BlockNbr: %d ", SDCardInfo.BlockNbr);                // 块数量
    log_i("Card BlockSize: %d ", SDCardInfo.BlockSize);              // 块大小
    log_i("LogBlockNbr: %d ", (uint32_t)(SDCardInfo.LogBlockNbr));   // 逻辑块数量
    log_i("LogBlockSize: %d ", (uint32_t)(SDCardInfo.LogBlockSize)); // 逻辑块大小
    log_i("Card Capacity: %d MB", (uint32_t)(CardCap >> 20));        // 卡容量
}
