/*
 * at25128b.c
 *
 *  Created on: Apr 6, 2026
 *      Author: vishnu-c-s
 */

#include "at25128b.h"

/* SPI timeout in milliseconds */
#define AT25128B_SPI_TIMEOUT_MS        (100U)
#define AT25128B_BUSY_TIMEOUT_MS       (15U)  /* Max write cycle time for AT25128B is 5ms; 15ms for margin */

static void at25128b_cs_enable(GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin);
static void at25128b_cs_disable(GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin);


/******************** Internal Helper Functions ********************/
static void at25128b_cs_enable(GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin)
{
    if (cs_gpiox == (void *)0) {
        return;
    }
   HAL_GPIO_WritePin(cs_gpiox, cs_gpio_pin, GPIO_PIN_RESET); /* Active LOW */
}

static void at25128b_cs_disable(GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin)
{
    if (cs_gpiox == (void *)0) {
        return;
    }
    HAL_GPIO_WritePin(cs_gpiox, cs_gpio_pin, GPIO_PIN_SET); /* Inactive HIGH */
}


HAL_StatusTypeDef at25128b_write_enable(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin)
{
    if ((hspi == (void *)0) || (cs_gpiox == (void *)0)) {
        return HAL_ERROR;
    }
    uint8_t cmd = AT25128B_CMD_WREN;
    HAL_StatusTypeDef status;

    at25128b_cs_enable(cs_gpiox, cs_gpio_pin);
    status = HAL_SPI_Transmit(hspi, &cmd, 1U, AT25128B_SPI_TIMEOUT_MS);
    at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
    if (status != HAL_OK) {
        return status;
    }

    cmd = AT25128B_CMD_RDSR;
    uint8_t status_reg = 0U;

    at25128b_cs_enable(cs_gpiox, cs_gpio_pin);
    status = HAL_SPI_Transmit(hspi, &cmd, 1U, AT25128B_SPI_TIMEOUT_MS);
    if (status != HAL_OK) {
        at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
        return status;
    }
    status = HAL_SPI_Receive(hspi, &status_reg, 1U, AT25128B_SPI_TIMEOUT_MS);
    at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
    if (status != HAL_OK) {
        return status;
    }

    if ((status_reg & AT25128B_SR_WEL) == 0U) {
        return HAL_ERROR; /* Write Enable Latch not set */
    }

    return HAL_OK;
}

HAL_StatusTypeDef at25128b_write_disable(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin)
{
    if ((hspi == (void *)0) || (cs_gpiox == (void *)0)) {
        return HAL_ERROR;
    }
    uint8_t cmd = AT25128B_CMD_WRDI;
    HAL_StatusTypeDef status;

    at25128b_cs_enable(cs_gpiox, cs_gpio_pin);
    status = HAL_SPI_Transmit(hspi, &cmd, 1U, AT25128B_SPI_TIMEOUT_MS);
    at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
    return status;
}

HAL_StatusTypeDef at25128b_write_uint8(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin, uint16_t address, const uint8_t* data)
{
    if ((hspi == (void *)0) || (cs_gpiox == (void *)0)) {
        return HAL_ERROR;
    }
    if (data == (void *)0) {
        return HAL_ERROR;
    }
    HAL_StatusTypeDef ret;
    uint8_t header[3];
    uint8_t status = 0U;
    uint32_t timeout_start;

    ret = at25128b_write_enable(hspi, cs_gpiox, cs_gpio_pin);
    if (ret != HAL_OK) {
        return ret;
    }

    header[0] = AT25128B_CMD_WRITE;
    header[1] = (uint8_t)(address >> 8U);
    header[2] = (uint8_t)(address & 0xFFU);

    at25128b_cs_enable(cs_gpiox, cs_gpio_pin);
    ret = HAL_SPI_Transmit(hspi, header, 3U, AT25128B_SPI_TIMEOUT_MS);
    if (ret != HAL_OK) {
        at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
        return ret;
    }
    ret = HAL_SPI_Transmit(hspi, data, 1U, AT25128B_SPI_TIMEOUT_MS);
    if (ret != HAL_OK) {
        at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
        return ret;
    }
    at25128b_cs_disable(cs_gpiox, cs_gpio_pin);

    timeout_start = HAL_GetTick();
    /* Wait for write cycle to complete (WIP bit cleared) */
    while (1) {
        uint8_t cmd = AT25128B_CMD_RDSR;
        at25128b_cs_enable(cs_gpiox, cs_gpio_pin);
        ret = HAL_SPI_Transmit(hspi, &cmd, 1U, AT25128B_SPI_TIMEOUT_MS);
        if (ret != HAL_OK) {
            at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
            return ret;
        }
        ret = HAL_SPI_Receive(hspi, &status, 1U, AT25128B_SPI_TIMEOUT_MS);
        at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
        if (ret != HAL_OK) {
            return ret;
        }
        /* Check WIP cleared BEFORE checking timeout so that a write completing
         * exactly at the timeout boundary is not incorrectly reported as TIMEOUT. */
        if ((status & AT25128B_SR_RDY_BSY) == 0U) {
            break;
        }
        if ((HAL_GetTick() - timeout_start) > AT25128B_BUSY_TIMEOUT_MS) {
            return HAL_TIMEOUT; /* Write cycle timeout */
        }
    }
    return HAL_OK;
}

HAL_StatusTypeDef at25128b_read_uint8(SPI_HandleTypeDef* hspi, GPIO_TypeDef* cs_gpiox, uint16_t cs_gpio_pin, uint16_t address, uint8_t* data)
{
    if ((hspi == (void *)0) || (cs_gpiox == (void *)0)) {
        return HAL_ERROR;
    }
    if (data == (void *)0) {
        return HAL_ERROR;
    }
    HAL_StatusTypeDef ret;
    uint8_t header[3];
    header[0] = AT25128B_CMD_READ;
    header[1] = (uint8_t)(address >> 8U);
    header[2] = (uint8_t)(address & 0xFFU);

    at25128b_cs_enable(cs_gpiox, cs_gpio_pin);
    ret = HAL_SPI_Transmit(hspi, header, 3U, AT25128B_SPI_TIMEOUT_MS);
    if (ret != HAL_OK) {
        at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
        return ret;
    }
    ret = HAL_SPI_Receive(hspi, data, 1U, AT25128B_SPI_TIMEOUT_MS);
    at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
    return ret;
}

/*
 * at25128b_write_page
 * Writes up to 64 bytes within a single page row (all bytes must be in the same row).
 * The AT25128B has a 64-byte page size. The six lowest-order address bits are
 * internally incremented after each byte; higher-order bits stay fixed.
 * Rising CS edge after the last byte triggers the internal write cycle.
 */
HAL_StatusTypeDef at25128b_write_page(SPI_HandleTypeDef* hspi,
                                      GPIO_TypeDef* cs_gpiox,
                                      uint16_t cs_gpio_pin,
                                      uint16_t address,
                                      const uint8_t* data,
                                      uint8_t length)
{
    if ((hspi == (void *)0) || (cs_gpiox == (void *)0) || (data == (void *)0)) {
        return HAL_ERROR;
    }
    if ((length == 0U) || (length > AT25128B_PAGE_SIZE)) {
        return HAL_ERROR;
    }
    /* Guard: all bytes must fit within the same 64-byte page row */
    if (((uint16_t)(address & 0x3FU) + (uint16_t)length) > (uint16_t)AT25128B_PAGE_SIZE) {
        return HAL_ERROR; /* Would cause address rollover within page */
    }
    if (((uint32_t)address + (uint32_t)length) > AT25128B_CAPACITY_BYTES) {
        return HAL_ERROR; /* Out of bounds */
    }

    HAL_StatusTypeDef ret;
    uint8_t header[3];
    uint8_t status = 0U;
    uint32_t timeout_start;

    ret = at25128b_write_enable(hspi, cs_gpiox, cs_gpio_pin);
    if (ret != HAL_OK) {
        return ret;
    }

    header[0] = AT25128B_CMD_WRITE;
    header[1] = (uint8_t)(address >> 8U);
    header[2] = (uint8_t)(address & 0xFFU);

    /* CS must remain LOW for entire header + data transmission */
    at25128b_cs_enable(cs_gpiox, cs_gpio_pin);
    ret = HAL_SPI_Transmit(hspi, header, 3U, AT25128B_SPI_TIMEOUT_MS);
    if (ret != HAL_OK) {
        at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
        return ret;
    }
    ret = HAL_SPI_Transmit(hspi, data, (uint16_t)length, AT25128B_SPI_TIMEOUT_MS);
    at25128b_cs_disable(cs_gpiox, cs_gpio_pin); /* Rising CS edge triggers internal write cycle */
    if (ret != HAL_OK) {
        return ret;
    }

    /* Poll WIP bit until write cycle completes */
    timeout_start = HAL_GetTick();
    while (1) {
        uint8_t cmd = AT25128B_CMD_RDSR;
        at25128b_cs_enable(cs_gpiox, cs_gpio_pin);
        ret = HAL_SPI_Transmit(hspi, &cmd, 1U, AT25128B_SPI_TIMEOUT_MS);
        if (ret != HAL_OK) {
            at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
            return ret;
        }
        ret = HAL_SPI_Receive(hspi, &status, 1U, AT25128B_SPI_TIMEOUT_MS);
        at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
        if (ret != HAL_OK) {
            return ret;
        }
        if ((status & AT25128B_SR_RDY_BSY) == 0U) {
            break; /* Write cycle complete */
        }
        if ((HAL_GetTick() - timeout_start) > AT25128B_BUSY_TIMEOUT_MS) {
            return HAL_TIMEOUT;
        }
    }

    return HAL_OK;
}

/*
 * at25128b_write_buffer
 * Writes arbitrary length data starting at address, automatically splitting
 * across 64-byte page boundaries. Each page chunk issues its own
 * WREN + write cycle + WIP poll sequence.
 */
HAL_StatusTypeDef at25128b_write_buffer(SPI_HandleTypeDef* hspi,
                                        GPIO_TypeDef* cs_gpiox,
                                        uint16_t cs_gpio_pin,
                                        uint16_t address,
                                        const uint8_t* data,
                                        uint16_t length)
{
    if ((hspi == (void *)0) || (cs_gpiox == (void *)0) || (data == (void *)0)) {
        return HAL_ERROR;
    }
    if (length == 0U) {
        return HAL_ERROR;
    }
    if (((uint32_t)address + (uint32_t)length) > AT25128B_CAPACITY_BYTES) {
        return HAL_ERROR; /* Out of bounds */
    }

    uint16_t written = 0U;

    while (written < length)
    {
        uint16_t current_addr  = address + written;
        uint8_t  page_offset   = (uint8_t)(current_addr & 0x3FU);                    /* Byte position within 64-byte row */
        uint8_t  space_in_page = (uint8_t)((uint16_t)AT25128B_PAGE_SIZE - (uint16_t)page_offset); /* Remaining space in this row */
        uint16_t remaining     = length - written;

        /* Write only what fits in this page row */
        uint8_t chunk = (uint8_t)((remaining < (uint16_t)space_in_page) ? remaining : space_in_page);

        HAL_StatusTypeDef ret = at25128b_write_page(hspi, cs_gpiox, cs_gpio_pin,
                                                     current_addr,
                                                     &data[written],
                                                     chunk);
        if (ret != HAL_OK) {
            return ret;
        }

        written += chunk;
    }

    return HAL_OK;
}

/*
 * at25128b_read_buffer
 * Sequential read of arbitrary length starting at address.
 * The AT25128B auto-increments the internal address pointer — no page
 * boundary restriction applies for reads.
 */
HAL_StatusTypeDef at25128b_read_buffer(SPI_HandleTypeDef* hspi,
                                       GPIO_TypeDef* cs_gpiox,
                                       uint16_t cs_gpio_pin,
                                       uint16_t address,
                                       uint8_t* data,
                                       uint16_t length)
{
    if ((hspi == (void *)0) || (cs_gpiox == (void *)0) || (data == (void *)0)) {
        return HAL_ERROR;
    }
    if (length == 0U) {
        return HAL_ERROR;
    }
    if (((uint32_t)address + (uint32_t)length) > AT25128B_CAPACITY_BYTES) {
        return HAL_ERROR; /* Out of bounds */
    }

    HAL_StatusTypeDef ret;
    uint8_t header[3];

    header[0] = AT25128B_CMD_READ;
    header[1] = (uint8_t)(address >> 8U);
    header[2] = (uint8_t)(address & 0xFFU);

    /* Hold CS low for entire read — address auto-increments on each clock */
    at25128b_cs_enable(cs_gpiox, cs_gpio_pin);
    ret = HAL_SPI_Transmit(hspi, header, 3U, AT25128B_SPI_TIMEOUT_MS);
    if (ret != HAL_OK) {
        at25128b_cs_disable(cs_gpiox, cs_gpio_pin);
        return ret;
    }
    ret = HAL_SPI_Receive(hspi, data, length, AT25128B_SPI_TIMEOUT_MS);
    at25128b_cs_disable(cs_gpiox, cs_gpio_pin);

    return ret;
}
