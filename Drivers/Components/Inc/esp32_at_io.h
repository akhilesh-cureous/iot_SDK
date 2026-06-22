/*
 * esp32_at_io.h
 *
 *  Created on: 16-Jun-2026
 *      Author: Akhilesh
 */

#ifndef COMPONENTS_INC_ESP32_AT_IO_H_
#define COMPONENTS_INC_ESP32_AT_IO_H_



#endif /* COMPONENTS_INC_ESP32_AT_IO_H_ */

#include "main.h"


#ifndef ESP32_UART_CONFIG
  extern   UART_HandleTypeDef   huart7;
  #define  ESP32_UART_HANDLE    &huart7
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define DEFAULT_TIME_OUT      7000  /* in ms */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
int8_t  esp32_io_init(void);
int8_t esp32_io_send(uint8_t *p_data, uint32_t length);
int32_t esp32_io_recv(uint8_t *buffer, uint32_t length);
