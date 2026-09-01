/*
 * oled.h
 *
 *  Created on: Aug 27, 2026
 *  Author: amukhopadhyay
 */

#ifndef INC_OLED_H_
#define INC_OLED_H_

#include "main.h"

HAL_StatusTypeDef OLED_Init(void);
void OLED_FillTest(void);
void OLED_Clear(void);
void OLED_UpdateScreen(void);
void OLED_DrawString(uint8_t x, uint8_t y, const char *text);

#endif /* INC_OLED_H_ */
