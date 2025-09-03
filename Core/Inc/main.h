/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.h
 * @brief          : Header for main.c file.
 *                   This file contains the common defines of the application.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "usart.h"
#include "spi.h"
#include "dma.h"
#include "spi.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"
#include "w25qxx.h"
#include "nvs_flash.h"
#include "MultiTimer.h"
#include "sys_time.h"
#include "sd_card.h"
#include "malloc.h"
#include "esp8266.h"
#include "ble.h"
#include "at24c02.h"
#include "iap.h"
#include "eth.h"
#include "msg_queue.h"
#include "ring_buf.h"
#include "led.h"
#include "buzzer.h"
#include "button.h"
#include "log.h"
#include "app.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
#define SoftWare_Version "V1.0.0"
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_0_Pin GPIO_PIN_3
#define LED_0_GPIO_Port GPIOE
#define LED_1_Pin GPIO_PIN_4
#define LED_1_GPIO_Port GPIOE
#define KEY_3_Pin GPIO_PIN_6
#define KEY_3_GPIO_Port GPIOF
#define KEY_2_Pin GPIO_PIN_7
#define KEY_2_GPIO_Port GPIOF
#define KEY_1_Pin GPIO_PIN_8
#define KEY_1_GPIO_Port GPIOF
#define KEY_0_Pin GPIO_PIN_9
#define KEY_0_GPIO_Port GPIOF
#define BLE_Connect_Pin GPIO_PIN_13
#define BLE_Connect_GPIO_Port GPIOF
#define ESP8266_EN_Pin GPIO_PIN_14
#define ESP8266_EN_GPIO_Port GPIOF
#define Buzzer_Pin GPIO_PIN_7
#define Buzzer_GPIO_Port GPIOG
#define W25Qxx_CS_Pin GPIO_PIN_8
#define W25Qxx_CS_GPIO_Port GPIOG
#define LED_2_Pin GPIO_PIN_9
#define LED_2_GPIO_Port GPIOG

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
