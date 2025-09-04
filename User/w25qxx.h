#ifndef __W25QXX_H__
#define __W25QXX_H__

#include "main.h"
#include "sfud.h"
#include "lfs_config.h"
#include "lfs.h"
#include "lfs_util.h"

// W25X系列/Q系列芯片列表
#define W25Q80 0XEF13
#define W25Q16 0XEF14
#define W25Q32 0XEF15
#define W25Q64 0XEF16
#define W25Q128 0XEF17

#define W25X_WriteEnable 0x06
#define W25X_WriteDisable 0x04
#define W25X_ReadStatusReg 0x05
#define W25X_WriteStatusReg 0x01
#define W25X_ReadData 0x03
#define W25X_FastReadData 0x0B
#define W25X_FastReadDual 0x3B
#define W25X_PageProgram 0x02
#define W25X_BlockErase 0xD8
#define W25X_SectorErase 0x20
#define W25X_ChipErase 0xC7
#define W25X_PowerDown 0xB9
#define W25X_ReleasePowerDown 0xAB
#define W25X_DeviceID 0xAB
#define W25X_ManufactDeviceID 0x90
#define W25X_JedecDeviceID 0x9F

// W25Q128 特定配置
#define W25Q128_FLASH_SIZE (16 * 1024 * 1024)                         // 16MB
#define W25Q128_SECTOR_SIZE 4096                                      // 4KB 扇区
#define W25Q128_SECTOR_NUM (W25Q128_FLASH_SIZE / W25Q128_SECTOR_SIZE) // 4096

typedef struct
{
    uint16_t id;
} w25qxx_device_t;

void user_lfs_init(void);

#endif /* __W25QXX_H__ */
