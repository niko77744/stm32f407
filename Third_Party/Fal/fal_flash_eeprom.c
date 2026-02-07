/*
 * Copyright (c) 2020, Armink, <armink.ztl@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <fal.h>

#include <stm32f4xx_hal.h>
#include "at24c02.h"

static int read(long offset, uint8_t *buf, size_t size)
{
    return at24c02_read(offset, buf, size);
}

static int write(long offset, const uint8_t *buf, size_t size)
{
    return at24c02_write(offset, (uint8_t *)buf, size);
}

static int erase(long offset, size_t size)
{
    return at24c02_erase(offset, size, 0xFF);
}

const struct fal_flash_dev eeprom =
    {
        .name = "eeprom",
        .addr = 0,
        .len = 256,
        .blk_size = 16,
        .ops = {NULL, read, write, erase},
        .write_gran = 8};
