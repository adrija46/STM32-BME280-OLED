/*
 * console.h
 *
 *  Created on: Sep 3, 2026
 *      Author: amukhopadhyay
 */

#ifndef INC_CONSOLE_H_
#define INC_CONSOLE_H_

#include "main.h"

#define CONSOLE_COMMAND_BUFFER_SIZE 64U

void Console_Init(void);
void Console_Print(const char *message);

uint8_t Console_CommandReady(void);
const char *Console_GetCommand(void);
void Console_ClearCommand(void);

#endif /* INC_CONSOLE_H_ */
