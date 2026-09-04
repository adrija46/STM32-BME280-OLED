/*
 * sensor.h
 *
 *  Created on: Sep 1, 2026
 *      Author: amukhopadhyay
 */

#ifndef INC_SENSOR_H_
#define INC_SENSOR_H_

#include "main.h"

typedef struct
{
    float temperature;
    float humidity;
    float pressure;
} SensorData;

HAL_StatusTypeDef Sensor_Init(void);
void Sensor_Process(void);

uint8_t Sensor_IsInitialized(void);
uint8_t Sensor_HasValidData(void);

void Sensor_GetLatestData(SensorData *data);

uint32_t Sensor_GetSampleInterval(void);
void Sensor_SetSampleInterval(uint32_t interval_ms);


#endif /* INC_SENSOR_H_ */
