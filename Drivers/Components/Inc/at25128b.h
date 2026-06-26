/*
 * at25128b.h
 *
 *  Created on: Apr 6, 2026
 *      Author: vishnu-c-s
 */

#ifndef DRIVERS_INC_AT25128B_H_
#define DRIVERS_INC_AT25128B_H_

#include <stdint.h>
#include "main.h"

#define AT25128B_SPI_TIMEOUT_MS        (100U)


/* AT25128B/AT25256B Instruction Set */
#define AT25128B_CMD_WREN  ((uint8_t)0x06)  /* 0000 0110: Set Write Enable Latch */
#define AT25128B_CMD_WRDI  ((uint8_t)0x04)  /* 0000 0100: Reset Write Enable Latch */
#define AT25128B_CMD_RDSR  ((uint8_t)0x05)  /* 0000 0101: Read STATUS Register */
#define AT25128B_CMD_WRSR  ((uint8_t)0x01)  /* 0000 0001: Write STATUS Register */
#define AT25128B_CMD_READ  ((uint8_t)0x03)  /* 0000 0011: Read from Memory Array */
#define AT25128B_CMD_WRITE ((uint8_t)0x02)  /* 0000 0010: Write to Memory Array */

/* Memory organisation */
#define AT25128B_PAGE_SIZE       ((uint8_t)64U)    /* 64-byte page row */
#define AT25128B_CAPACITY_BYTES  ((uint32_t)16384U) /* 128 Kbit = 16384 bytes */

/* STATUS Register Bit Masks */
#define AT25128B_SR_RDY_BSY ((uint8_t)(1U << 0))  /* Bit 0: 1 = Busy, 0 = Ready */
#define AT25128B_SR_WEL     ((uint8_t)(1U << 1))  /* Bit 1: 1 = Write Enabled */
#define AT25128B_SR_BP0     ((uint8_t)(1U << 2))  /* Bit 2: Block Protection 0 */
#define AT25128B_SR_BP1     ((uint8_t)(1U << 3))  /* Bit 3: Block Protection 1 */
#define AT25128B_SR_WPEN    ((uint8_t)(1U << 7))  /* Bit 7: Write-Protect Enable */

/* Function Prototypes */
HAL_StatusTypeDef at25128b_write_enable(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin);
HAL_StatusTypeDef at25128b_write_disable(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin);
HAL_StatusTypeDef at25128b_write_uint8(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin, uint16_t address, const uint8_t* data);
HAL_StatusTypeDef at25128b_read_uint8(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin, uint16_t address, uint8_t* data);
HAL_StatusTypeDef at25128b_write_page(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin, uint16_t address, const uint8_t* data, uint8_t length);
HAL_StatusTypeDef at25128b_write_buffer(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin, uint16_t address, const uint8_t* data, uint16_t length);
HAL_StatusTypeDef at25128b_read_buffer(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin, uint16_t address, uint8_t* data, uint16_t length);

#endif /* DRIVERS_INC_AT25128B_H_ */
