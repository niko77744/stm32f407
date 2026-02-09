#ifndef __BLE_H__
#define __BLE_H__

#include "main.h"

#define BLE_BUFFER_SIZE 32
extern uint8_t ble_dma_rx_buffer[BLE_BUFFER_SIZE] __attribute__((aligned(4)));
void ble_init(void);
void ble_rx_event_callback(uint16_t Size);
#endif /* __BLE_H__ */
