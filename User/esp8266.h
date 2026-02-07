#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "main.h"

void esp8266_hw_init(void);
void wifi_rx_event_callback(uint16_t Size);
extern uint8_t esp8266_buf[256];

#endif /* __ESP8266_H__ */
