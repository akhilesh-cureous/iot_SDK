/*
 * esp32_at_io.c
 *
 *  Created on: 16-Jun-2026
 *      Author: Akhilesh
 */

/* Includes ------------------------------------------------------------------*/
#include "esp32_at_io.h"
#include "main.h"
#include <string.h>

/* Private define ------------------------------------------------------------*/
#define RING_BUFFER_SIZE (1024 * 10)

/* Private typedef -----------------------------------------------------------*/
typedef struct {
  uint8_t data [ RING_BUFFER_SIZE ];
  uint16_t tail;
  uint16_t head;
} ring_buffer_t;

/* Private variables ---------------------------------------------------------*/
ring_buffer_t wifi_rx_buffer;



int8_t esp32_io_init(void) {
  HAL_Delay(2000);

  wifi_rx_buffer.head = 0;
  wifi_rx_buffer.tail = 0;

  // Start UART in DMA mode with Idle line detection
  if ( HAL_UARTEx_ReceiveToIdle_DMA(ESP32_UART_HANDLE, (uint8_t *)wifi_rx_buffer.data,
                                    RING_BUFFER_SIZE) != HAL_OK ) {
    return -1;
  }
  return 0;
}



int8_t esp32_io_send(uint8_t *p_data, uint32_t length) {
  if ( HAL_UART_Transmit(ESP32_UART_HANDLE, p_data, length, DEFAULT_TIME_OUT) !=
       HAL_OK ) {
    return -1;
  }
  return 0;
}

/**
 * @brief  Receive data from the ESP8266 module over UART.
 * @param  buffer: Pointer to the buffer to store received data.
 * @param  length: Maximum length of the buffer.
 * @retval Number of bytes received.
 */

int32_t esp32_io_recv(uint8_t *buffer, uint32_t length) {
  uint32_t read_data = 0;

  while ( length-- ) {
    uint32_t tick_start = HAL_GetTick();
    do {
      if ( wifi_rx_buffer.head != wifi_rx_buffer.tail ) {
        *buffer++ = wifi_rx_buffer.data [ wifi_rx_buffer.head++ ];
        read_data++;

        if ( wifi_rx_buffer.head >= RING_BUFFER_SIZE ) {
          wifi_rx_buffer.head = 0;
        }
        break;
      }
    } while ( (HAL_GetTick() - tick_start) < DEFAULT_TIME_OUT);
  }
  return read_data;
}

