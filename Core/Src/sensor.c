/*
 * sensor.c
 *
 *  Created on: Sep 1, 2026
 *      Author: amukhopadhyay
 */

#include "sensor.h"
#include "bme280.h"
#include <string.h>

#include "i2c_if.h"

static struct bme280_dev bme280_device;
static uint8_t bme280_i2c_address = BME280_I2C_ADDR_PRIM;
static uint8_t bme280_initialized = 0U;

static struct bme280_data latest_bme280_data;

static uint32_t last_sample_time_ms = 0;
static uint32_t sample_interval_ms = 1000U;
static uint8_t sample_valid = 0U;

static BME280_INTF_RET_TYPE STM32_BME280_Read(
    uint8_t reg_addr,
    uint8_t *reg_data,
    uint32_t length,
    void *intf_ptr);

static BME280_INTF_RET_TYPE STM32_BME280_Write(
    uint8_t reg_addr,
    const uint8_t *reg_data,
    uint32_t length,
    void *intf_ptr);

static void STM32_BME280_DelayUs(
    uint32_t period,
    void *intf_ptr);

static int8_t BME280_AppInit(void);
static void BME280_SamplingProcess(void);

uint8_t Sensor_IsInitialized(void)
{
    return bme280_initialized;
}

uint8_t Sensor_HasValidData(void)
{
    return sample_valid;
}

void Sensor_Process(void)
{
    BME280_SamplingProcess();
}

void Sensor_GetLatestData(SensorData *data)
{
    if (data == NULL)
    {
        return;
    }

    data->temperature = latest_bme280_data.temperature;
    data->humidity = latest_bme280_data.humidity;
    data->pressure = latest_bme280_data.pressure;
}

uint32_t Sensor_GetSampleInterval(void)
{
    return sample_interval_ms;
}

void Sensor_SetSampleInterval(uint32_t interval_ms)
{
    sample_interval_ms = interval_ms;
}

static BME280_INTF_RET_TYPE STM32_BME280_Read(
    uint8_t reg_addr,
    uint8_t *reg_data,
    uint32_t length,
    void *intf_ptr)
{
    if ((reg_data == NULL) || (intf_ptr == NULL))
    {
        return BME280_E_NULL_PTR;
    }

    uint8_t device_address = *(uint8_t *)intf_ptr;

    HAL_StatusTypeDef status = I2C_IF_ReadRegister(
        device_address,
        reg_addr,
        reg_data,
        (uint16_t)length,
        100U
    );

    return (status == HAL_OK) ? BME280_INTF_RET_SUCCESS : BME280_E_COMM_FAIL;
}

static BME280_INTF_RET_TYPE STM32_BME280_Write(
    uint8_t reg_addr,
    const uint8_t *reg_data,
    uint32_t length,
    void *intf_ptr)
{
    if ((reg_data == NULL) || (intf_ptr == NULL))
    {
        return BME280_E_NULL_PTR;
    }

    uint8_t device_address = *(uint8_t *)intf_ptr;

    HAL_StatusTypeDef status = I2C_IF_WriteRegister(
        device_address,
        reg_addr,
        reg_data,
        (uint16_t)length,
        100U
    );

    return (status == HAL_OK) ? BME280_INTF_RET_SUCCESS : BME280_E_COMM_FAIL;
}

static void STM32_BME280_DelayUs(
    uint32_t period,
    void *intf_ptr)
{
    (void)intf_ptr;

    uint32_t delay_ms = (period + 999U) / 1000U;

    if (delay_ms == 0U)
    {
        delay_ms = 1U;
    }

    HAL_Delay(delay_ms);
}

static int8_t BME280_AppInit(void)
{
    int8_t result;
    struct bme280_settings settings;

    memset(&bme280_device, 0, sizeof(bme280_device));
    memset(&settings, 0, sizeof(settings));

    bme280_device.intf = BME280_I2C_INTF;
    bme280_device.intf_ptr = &bme280_i2c_address;
    bme280_device.read = STM32_BME280_Read;
    bme280_device.write = STM32_BME280_Write;
    bme280_device.delay_us = STM32_BME280_DelayUs;

    result = bme280_init(&bme280_device);

    if (result != BME280_OK)
    {
       // UART_Print("BME280 driver initialization failed.\r\n");
        return result;
    }

    settings.filter = BME280_FILTER_COEFF_OFF;
    settings.osr_h = BME280_OVERSAMPLING_1X;
    settings.osr_p = BME280_OVERSAMPLING_1X;
    settings.osr_t = BME280_OVERSAMPLING_1X;
    settings.standby_time = BME280_STANDBY_TIME_1000_MS;

    result = bme280_set_sensor_settings(
        BME280_SEL_ALL_SETTINGS,
        &settings,
        &bme280_device
    );

    if (result != BME280_OK)
    {
       // UART_Print("BME280 settings configuration failed.\r\n");
        return result;
    }

    result = bme280_set_sensor_mode(
        BME280_POWERMODE_NORMAL, // In normal mode it automatically cycles between measurement and standby. FW then periodically reads the most recently available measurement.
        &bme280_device
    );

    if (result != BME280_OK)
    {
       // UART_Print("BME280 mode configuration failed.\r\n");
        return result;
    }

    bme280_initialized = 1;
    //UART_Print("BME280 initialized successfully.\r\n");

    return BME280_OK;
}

HAL_StatusTypeDef Sensor_Init(void)
{
    int8_t result = BME280_AppInit();

    if (result == BME280_OK)
    {
        return HAL_OK;
    }

    return HAL_ERROR;
}

/*
 * @brief verify initialization
 * check whether the interval has elapsed
 * read the latest sensor data
 */
static void BME280_SamplingProcess(void)
{
    if (!bme280_initialized)
    {
        return;
    }

    uint32_t current_time_ms = HAL_GetTick();

    if ((current_time_ms - last_sample_time_ms) < sample_interval_ms)
    {
        return;
    }

    last_sample_time_ms = current_time_ms;

    int8_t result = bme280_get_sensor_data(
        BME280_ALL,
        &latest_bme280_data,
        &bme280_device
    );

    if (result == BME280_OK)
    {
        sample_valid = 1;
    }
    else
    {
        sample_valid = 0;
    }
}

