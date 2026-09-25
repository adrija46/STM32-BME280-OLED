/*
 * i2c_if.h
 *
 *  Created on: Sep 20, 2026
 *      Author: amukhopadhyay
 */

#ifndef INC_I2C_IF_H_
#define INC_I2C_IF_H_

#include "main.h"

HAL_StatusTypeDef I2C_IF_ReadRegister(
    uint8_t device_address,
    uint8_t register_address,
    uint8_t *data,
    uint16_t length,
    uint32_t timeout_ms
);

HAL_StatusTypeDef I2C_IF_WriteRegister(
    uint8_t device_address,
    uint8_t register_address,
    const uint8_t *data,
    uint16_t length,
    uint32_t timeout_ms
);

HAL_StatusTypeDef I2C_IF_IsDeviceReady(
    uint8_t device_address,
    uint32_t trials,
    uint32_t timeout_ms
);

#endif /* INC_I2C_IF_H_ */
