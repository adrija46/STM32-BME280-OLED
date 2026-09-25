/*
 * uart_if.h
 *
 *  Created on: Sep 24, 2026
 *      Author: amukhopadhyay
 */

#ifndef INC_UART_IF_H_
#define INC_UART_IF_H_

#include "main.h"

HAL_StatusTypeDef UART_IF_Transmit(
    const uint8_t *data,
    uint16_t length,
    uint32_t timeout_ms
);

HAL_StatusTypeDef UART_IF_ReceiveIT(
    uint8_t *data,
    uint16_t length
);

uint8_t UART_IF_IsConsoleUART(
    UART_HandleTypeDef *huart
);


#endif /* INC_UART_IF_H_ */
