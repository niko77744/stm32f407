#define LOG_TAG "sd_card"
#include "sd_card.h"
#include "elog.h"
#include "fatfs.h"
#include "malloc.h"
#include "string.h"

// �õ�����Ϣ
// cardinfo:����Ϣ�洢��
// ����ֵ:����״̬
uint8_t SD_GetCardInfo(HAL_SD_CardInfoTypeDef *cardinfo)
{
    uint8_t sta;
    sta = HAL_SD_GetCardInfo(&hsd, cardinfo);
    return sta;
}

// ��ȡ����״̬
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

// SD������������������Ϊ��С�����ж�ȡ�ġ�Ҳ����SD������ÿ����ַ��512���ֽڡ�������С��д��Ԫ����512���ֽڣ�Ҳ����һ��������
uint8_t SD_ReadBlocks_DMA(uint32_t *buf, uint64_t sector, uint32_t blocksize, uint32_t cnt)
{
    uint8_t err = 0;
    uint32_t timeout = SD_TIMEOUT;
    err = HAL_SD_ReadBlocks_DMA(&hsd, (uint8_t *)buf, sector, cnt); // ͨ��DMA��ȡSD��n������

    if (err == 0)
    {
        // �ȴ�SD������
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
    err = HAL_SD_WriteBlocks_DMA(&hsd, (uint8_t *)buf, sector, cnt); // ͨ��DMAдSD��n������

    if (err == 0)
    {
        // �ȴ�SD��д��
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

    sta = SD_WriteBlocks_DMA((uint32_t *)buf, lsector, 512, cnt); // ���sector��д����

    return sta;
}

uint8_t SD_ReadSDisk_NoDma(uint8_t *buf, uint32_t sector, uint32_t cnt)
{
    uint8_t sta = HAL_OK;
    uint32_t timeout = SD_TIMEOUT;
    long long lsector = sector;
    __disable_irq();                                                         // �ر����ж�
    sta = HAL_SD_ReadBlocks(&hsd, (uint8_t *)buf, lsector, cnt, SD_TIMEOUT); // ���sector�Ķ�����

    // �ȴ�SD������
    while (SD_GetCardState() != SD_TRANSFER_OK)
    {
        if (timeout-- == 0)
        {
            sta = SD_TRANSFER_BUSY;
        }
    }
    __enable_irq(); // �������ж�
    return sta;
}

uint8_t SD_WriteSDisk_NoDma(uint8_t *buf, uint32_t sector, uint32_t cnt)
{
    uint8_t sta = HAL_OK;
    uint32_t timeout = SD_TIMEOUT;
    long long lsector = sector;
    __disable_irq();                                                          // �ر����ж�(POLLINGģʽ,�Ͻ��жϴ��SDIO��д����!!!)
    sta = HAL_SD_WriteBlocks(&hsd, (uint8_t *)buf, lsector, cnt, SD_TIMEOUT); // ���sector��д����

    // �ȴ�SD��д��
    while (SD_GetCardState() != SD_TRANSFER_OK)
    {
        if (timeout-- == 0)
        {
            sta = SD_TRANSFER_BUSY;
        }
    }
    __enable_irq(); // �������ж�
    return sta;
}

void log_sd_card_info(void)
{
    uint64_t CardCap; // SD������
    HAL_SD_CardCIDTypeDef SDCard_CID;
    HAL_SD_CardInfoTypeDef SDCardInfo;

    HAL_SD_GetCardCID(&hsd, &SDCard_CID);                                               // ��ȡCID
    HAL_SD_GetCardInfo(&hsd, &SDCardInfo);                                              // ��ȡSD����Ϣ
    CardCap = (uint64_t)(SDCardInfo.LogBlockNbr) * (uint64_t)(SDCardInfo.LogBlockSize); // ����SD������
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

    log_i("Card ManufacturerID: %d", SDCard_CID.ManufacturerID);   // ������ID
    log_i("CardVersion: %d ", (uint32_t)(SDCardInfo.CardVersion)); // ���汾��
    log_i("Class: %d ", (uint32_t)(SDCardInfo.Class));
    log_i("Card RCA(RelCardAdd):%d ", SDCardInfo.RelCardAdd);        // ����Ե�ַ
    log_i("Card BlockNbr: %d ", SDCardInfo.BlockNbr);                // ������
    log_i("Card BlockSize: %d ", SDCardInfo.BlockSize);              // ���С
    log_i("LogBlockNbr: %d ", (uint32_t)(SDCardInfo.LogBlockNbr));   // �߼�������
    log_i("LogBlockSize: %d ", (uint32_t)(SDCardInfo.LogBlockSize)); // �߼����С
    log_i("Card Capacity: %d MB", (uint32_t)(CardCap >> 20));        // ������
}

void sd_fatfs_demo(void)
{
    FIL file;
    FRESULT res;
    FATFS *fsTF;
    UINT byteswritten, bytesread;
    char buffer[64];
    const char *text = "Hello, FatFS! This is a test file.";
    FILINFO fno;
    DIR dir;

    fsTF = (FATFS *)Mem_malloc(INSRAM, sizeof(FATFS)); // Ϊ�ļ�ϵͳ�����ڴ�

    // 1. �����ļ�ϵͳ
    res = f_mount(fsTF, "0:", 1);
    if (res != FR_OK)
    {
        log_e("Error mounting filesystem: %d", res);
        return;
    }

    log_sd_card_info();

    log_i("Filesystem mounted successfully!");

    // 2. ������д���ļ�
    log_i("Creating and writing to test.txt...");
    res = f_open(&file, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
        log_e("Error creating file: %d", res);
        f_mount(NULL, "", 0); // ж���ļ�ϵͳ
        return;
    }

    // д��һЩ�ı����ļ�
    res = f_write(&file, text, strlen(text), &byteswritten);
    if (res != FR_OK || byteswritten != strlen(text))
    {
        log_e("Error writing to file: %d", res);
        f_close(&file);
        f_mount(NULL, "", 0);
    }

    // �ر��ļ�
    f_close(&file);
    log_i("File written successfully! (%d bytes)", byteswritten);

    // 3. ��ȡ�ļ�����
    log_i("Reading file contents...");
    res = f_open(&file, "test.txt", FA_READ);
    if (res != FR_OK)
    {
        log_e("Error opening file for reading: %d", res);
        f_mount(NULL, "", 0);
        return;
    }

    // ��ȡ�ļ�����
    res = f_read(&file, buffer, sizeof(buffer) - 1, &bytesread);
    if (res != FR_OK)
    {
        log_e("Error reading file: %d", res);
        f_close(&file);
        f_mount(NULL, "", 0);
    }

    // ����ַ�������������ӡ����
    buffer[bytesread] = '\0';
    log_i("File contents: %s", buffer);

    // �ر��ļ�
    f_close(&file);

    // 4. ��ȡ�ļ���Ϣ
    log_i("Getting file info...");

    res = f_stat("test.txt", &fno);
    if (res == FR_OK)
    {
        log_i("File size: %lu bytes", fno.fsize);
        log_i("Timestamp: %u/%02u/%02u, %02u:%02u",
              (fno.fdate >> 9) + 1980, (fno.fdate >> 5) & 15, fno.fdate & 31,
              fno.ftime >> 11, (fno.ftime >> 5) & 63);
    }
    else
    {
        log_e("Error getting file info: %d", res);
        return;
    }

    // 5. �г���Ŀ¼���ݣ���ѡ��
    log_i("Listing root directory...");
    res = f_opendir(&dir, "/");
    if (res == FR_OK)
    {
        while (1)
        {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0)
                break;
            if (fno.fattrib & AM_DIR)
            {
                log_i("  [DIR]  %s", fno.fname);
            }
            else
            {
                log_i("  [FILE] %s (%lu bytes)", fno.fname, fno.fsize);
            }
        }
        f_closedir(&dir);
    }
    else
    {
        log_e("Error opening directory: %d", res);
        return;
    }

    // 6. ж���ļ�ϵͳ
    f_mount(NULL, "", 0);
    log_i("Demo completed successfully!");
    Mem_free(INSRAM, fsTF);
}
