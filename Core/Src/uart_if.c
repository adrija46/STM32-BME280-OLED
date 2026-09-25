/*
 * uart_if.c
 *
 *  Created on: Sep 24, 2026
 *      Author: amukhopadhyay
 */


#include "uart_if.h"

extern UART_HandleTypeDef huart2;

HAL_StatusTypeDef UART_IF_Transmit(
    const uint8_t *data,
    uint16_t length,
    uint32_t timeout_ms
)
{
    if ((data == NULL) || (length == 0U))
    {
        return HAL_ERROR;
    }

    return HAL_UART_Transmit(
        &huart2,
        (uint8_t *)data,
        length,
        timeout_ms
    );
}


HAL_StatusTypeDef UART_IF_ReceiveIT(
    uint8_t *data,
    uint16_t length
)
{
    if ((data == NULL) || (length == 0U))
    {
        return HAL_ERROR;
    }

    return HAL_UART_Receive_IT(
        &huart2,
        data,
        length
    );
}


uint8_t UART_IF_IsConsoleUART(
    UART_HandleTypeDef *huart
)
{
    if (huart == NULL)
    {
        return 0U;
    }

    return (huart->Instance == USART2) ? 1U : 0U;
}
