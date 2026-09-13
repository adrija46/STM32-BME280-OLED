# STM32 BME280 Environmental Monitor

A modular embedded firmware project built on the **STM32 NUCLEO-F303RE** using a **BME280 environmental sensor**, **SPI OLED display**, and **UART console**.

The goal of this project is not only to display temperature, humidity, and pressure, but also to practice real embedded firmware development: peripheral integration, driver abstraction, modular architecture, debugging, timing, and Git-based development.

## Current Features

- Temperature, humidity, and pressure measurement with BME280
- BME280 communication over I2C
- 0.96" OLED display over SPI
- UART console/debug output
- Periodic sensor sampling
- UART command handling
- Optional periodic UART sensor streaming
- Modularized OLED, sensor, and console code
- Hardware-tested on STM32 NUCLEO-F303RE

## Hardware

- STM32 NUCLEO-F303RE
- Bosch BME280 environmental sensor
- 0.96" SPI OLED display
- Breadboard and jumper wires
- ST-LINK debugger/programmer

## Connections

### BME280 - I2C1

| BME280 | STM32F303RE |
|---|---|
| VCC | 3.3 V |
| GND | GND |
| SCL | PB6 |
| SDA | PB7 |

The sensor is detected at I2C address `0x76`.

### OLED - SPI1

| OLED | STM32F303RE |
|---|---|
| SCK | PB13 |
| MOSI | PB15 |
| CS | PB12 |
| DC | PA8 |
| RESET | PA9 |
| VCC | 3.3 V |
| GND | GND |

## Firmware Architecture

The firmware has been progressively refactored from a monolithic `main.c` into separate modules.

```text
Core/
├── Inc/
│   ├── console.h
│   ├── oled.h
│   ├── sensor.h
│   ├── bme280.h
│   └── bme280_defs.h
└── Src/
    ├── console.c
    ├── oled.c
    ├── sensor.c
    ├── bme280.c
    └── main.c
```

### `main.c`

Responsible for system startup, HAL initialization, peripheral initialization, and coordinating the main application loop.

### `sensor.c / sensor.h`

Provides the application-facing BME280 abstraction.

Responsibilities include:

- BME280 initialization
- Periodic sensor sampling
- Tracking whether valid sensor data is available
- Storing and returning the latest sensor values
- Configurable sample interval

The public sensor data type is:

```c
typedef struct
{
    float temperature;
    float humidity;
    float pressure;
} SensorData;
```

### `oled.c / oled.h`

Contains the SPI OLED driver and rendering functions.

### `console.c / console.h`

Contains UART console functionality and separates UART handling from the main application code.

## Application Flow

```text
Power On
   |
   v
HAL + Clock Initialization
   |
   v
GPIO / I2C / UART / SPI Initialization
   |
   v
I2C Device Scan
   |
   v
BME280 Initialization
   |
   v
UART Console Initialization
   |
   v
OLED Initialization
   |
   v
Main Loop
   |
   +--> Sensor_Process()
   |
   +--> Process UART commands
   |
   +--> Stream sensor data when enabled
   |
   +--> Refresh OLED
   |
   +--> Repeat
```

## Development Environment

- STM32CubeIDE
- STM32CubeMX-generated HAL configuration
- STM32 HAL drivers
- GCC ARM Embedded toolchain
- Git / GitHub

Target MCU:

- STM32F303RE
- ARM Cortex-M4

## Validation

The current firmware has been tested on physical hardware and verified for:

- Successful build
- BME280 initialization
- Temperature readings
- Humidity readings
- Pressure readings
- OLED updates
- UART output
- UART command handling
- Periodic sensor sampling

The current tested build runs with **0 errors and 0 warnings**.

## Refactoring History

The project is being improved incrementally using feature branches and pull requests.

Completed refactors include:

1. OLED driver extracted from `main.c`
2. BME280 logic moved behind a sensor abstraction
3. UART console handling moved into a separate console module

This keeps the firmware easier to understand, test, and extend.

## Planned Improvements

- Complete the application-layer abstraction
- Improve sensor error handling and recovery
- Add system status / sensor health reporting
- Add min/max environmental values
- Add system uptime
- Improve non-blocking and interrupt-driven behavior
- Measure firmware latency using GPIO instrumentation and a logic analyzer
- Explore BACnet / building-automation integration

## What I Am Learning

This project is being used to develop practical skills in:

- Embedded C
- STM32 HAL
- GPIO
- UART
- I2C
- SPI
- Sensor integration
- Datasheet interpretation
- Firmware architecture
- Driver abstraction
- Hardware/software debugging
- Git branching and pull-request workflows
- Timing and latency measurement

## Project Status

**Active development.**

The core environmental monitor is working on hardware. The next phase focuses on improving architecture, robustness, and measurement/debugging techniques rather than simply adding display features.

## Author

Adrija Mukhopadhyay
