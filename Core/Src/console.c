/*
 * console.c
 *
 *  Created on: Sep 3, 2026
 *      Author: amukhopadhyay
 */

#include "console.h"

#include <string.h>

extern UART_HandleTypeDef huart2;

#define CONSOLE_COMMAND_BUFFER_SIZE 64U

static uint8_t uart_rx_byte;
static char command_buffer[CONSOLE_COMMAND_BUFFER_SIZE];
static volatile uint16_t command_index = 0U;
static volatile uint8_t command_ready = 0U;

void Console_Print(const char *message)
{
    if (message == NULL)
    {
        return;
    }

    HAL_UART_Transmit(
        &huart2,
        (uint8_t *)message,
        strlen(message),
        HAL_MAX_DELAY
    );
}

void Console_Init(void)
{
    HAL_UART_Receive_IT(
        &huart2,
        &uart_rx_byte,
        1
    );
}

uint8_t Console_CommandReady(void)
{
    return command_ready;
}

const char *Console_GetCommand(void)
{
    return command_buffer;
}

void Console_ClearCommand(void)
{
    command_index = 0U;
    command_ready = 0U;
    command_buffer[0] = '\0';
}

/**
 * @brief UART receive-complete callback, fires once per received byte.
 *
 * Called automatically by the HAL whenever a single byte requested by
 * HAL_UART_Receive_IT() has arrived. Runs in interrupt context.
 *
 * Builds up uart_command_buffer one character at a time until Enter
 * is pressed, then marks uart_command_ready so the main loop can
 * process the completed command in UART_CommandProcess().
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if ((uart_rx_byte == '\r') || (uart_rx_byte == '\n'))
        {
            if ((command_index > 0U) && (command_ready == 0U))
            {
                command_buffer[command_index] = '\0';
                command_ready = 1U;
            }
        }
        else
        {
            if (command_index < (CONSOLE_COMMAND_BUFFER_SIZE - 1U))
            {
                command_buffer[command_index] = (char)uart_rx_byte;
                command_index++;
            }
        }

        HAL_UART_Receive_IT(
            &huart2,
            &uart_rx_byte,
            1
        );
    }
}
