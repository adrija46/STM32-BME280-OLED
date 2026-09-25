/*
 * spi_if.h
 *
 *  Created on: Sep 24, 2026
 *      Author: amukhopadhyay
 */

#ifndef INC_SPI_IF_H_
#define INC_SPI_IF_H_
#include "main.h"

HAL_StatusTypeDef SPI_IF_Transmit(
    const uint8_t *data,
    uint16_t length,
    uint32_t timeout_ms
);


#endif /* INC_SPI_IF_H_ */
