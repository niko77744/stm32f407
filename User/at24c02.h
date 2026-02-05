#ifndef __AT24C02_H__
#define __AT24C02_H__

#include "main.h"

uint16_t at24c02_erase(uint16_t addr, uint16_t len, uint8_t erase_value);
uint16_t at24c02_write(uint8_t addr, uint8_t *data, uint16_t len);
uint16_t at24c02_read(uint8_t addr, uint8_t *data, uint16_t len);

#endif /* __AT24C02_H__ */
