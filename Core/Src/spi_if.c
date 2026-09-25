/*
 * spi_if.c
 *
 *  Created on: Sep 24, 2026
 *      Author: amukhopadhyay
 */


#include "spi_if.h"

extern SPI_HandleTypeDef hspi1;

HAL_StatusTypeDef SPI_IF_Transmit(
    const uint8_t *data,
    uint16_t length,
    uint32_t timeout_ms
)
{
    if ((data == NULL) || (length == 0U))
    {
        return HAL_ERROR;
    }

    return HAL_SPI_Transmit(
        &hspi1,
        (uint8_t *)data,
        length,
        timeout_ms
    );
}
