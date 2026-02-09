#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "main.h"

void esp8266_hw_init(void);
void wifi_rx_event_callback(uint16_t Size);

#define ESP_RX_LEN 256
extern uint8_t esp8266_dma_rx_buffer[ESP_RX_LEN] __attribute__((aligned(4)));

#endif /* __ESP8266_H__ */
