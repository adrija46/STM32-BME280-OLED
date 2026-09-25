/*
 * i2c_if.c
 *
 *  Created on: Sep 20, 2026
 *      Author: amukhopadhyay
 */

#include "i2c_if.h"

extern I2C_HandleTypeDef hi2c1;

HAL_StatusTypeDef I2C_IF_ReadRegister(
    uint8_t device_address,
    uint8_t register_address,
    uint8_t *data,
    uint16_t length,
    uint32_t timeout_ms
)
{
    if ((data == NULL) || (length == 0U))
    {
        return HAL_ERROR;
    }

    return HAL_I2C_Mem_Read(
        &hi2c1,
        device_address << 1,
        register_address,
        I2C_MEMADD_SIZE_8BIT,
        data,
        length,
        timeout_ms
    );
}


HAL_StatusTypeDef I2C_IF_WriteRegister(
    uint8_t device_address,
    uint8_t register_address,
    const uint8_t *data,
    uint16_t length,
    uint32_t timeout_ms
)
{
    if ((data == NULL) || (length == 0U))
    {
        return HAL_ERROR;
    }

    return HAL_I2C_Mem_Write(
        &hi2c1,
        device_address << 1,
        register_address,
        I2C_MEMADD_SIZE_8BIT,
        (uint8_t *)data,
        length,
        timeout_ms
    );
}


HAL_StatusTypeDef I2C_IF_IsDeviceReady(
    uint8_t device_address,
    uint32_t trials,
    uint32_t timeout_ms
)
{
    return HAL_I2C_IsDeviceReady(
        &hi2c1,
        device_address << 1,
        trials,
        timeout_ms
    );
}
