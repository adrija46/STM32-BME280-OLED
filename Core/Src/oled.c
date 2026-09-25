/*
 * oled.c
 *
 *  Created on: Aug 27, 2026
 *  Author: amukhopadhyay
 */


#include "oled.h"
#include <string.h>
#include "spi_if.h"

/*
 * OLED framebuffer.
 * 128 x 64 monochrome pixels:
 * 128 * 64 = 8192 pixels
 * Each byte stores 8 pixels:
 * 8192 / 8 = 1024 bytes
 */
static uint8_t oled_buffer[1024];

static void OLED_Reset(void);
static HAL_StatusTypeDef OLED_WriteCommand(uint8_t command);
static HAL_StatusTypeDef OLED_WriteData(uint8_t *data, uint16_t length);
static uint8_t OLED_GetFontColumn(char c, uint8_t column);
static void OLED_SetPixel(uint8_t x, uint8_t y);
static void OLED_DrawChar(uint8_t x, uint8_t y, char character);

static void OLED_Reset(void)
{
    /* Hold reset low. */
    HAL_GPIO_WritePin(
        OLED_RES_GPIO_Port,
        OLED_RES_Pin,
        GPIO_PIN_RESET
    );

    HAL_Delay(20);

    /* Release the OLED from reset. */
    HAL_GPIO_WritePin(
        OLED_RES_GPIO_Port,
        OLED_RES_Pin,
        GPIO_PIN_SET
    );

    HAL_Delay(20);
}

/**
 * @brief Sends one command byte to the OLED over SPI.
 *
 * DC = LOW tells the OLED that the byte is a command.
 * CS = LOW selects the OLED on the SPI bus.
 */
static HAL_StatusTypeDef OLED_WriteCommand(uint8_t command)
{
    HAL_StatusTypeDef status;

    /* Select OLED. */
    HAL_GPIO_WritePin(
        OLED_CS_GPIO_Port,
        OLED_CS_Pin,
        GPIO_PIN_RESET
    );

    /* Command mode. */
    HAL_GPIO_WritePin(
        OLED_DC_GPIO_Port,
        OLED_DC_Pin,
        GPIO_PIN_RESET
    );

    /* Transmit one byte using SPI1. */
    status = SPI_IF_Transmit(
        &command,
        1U,
        100U
    );

    /* Release OLED. */
    HAL_GPIO_WritePin(
        OLED_CS_GPIO_Port,
        OLED_CS_Pin,
        GPIO_PIN_SET
    );

    return status;
}

/**
 * @brief Sends display data bytes to the OLED.
 *
 * DC = HIGH tells the OLED that the bytes represent display data.
 */
static HAL_StatusTypeDef OLED_WriteData(
    uint8_t *data,
    uint16_t length)
{
    HAL_StatusTypeDef status;

    if ((data == NULL) || (length == 0U))
    {
        return HAL_ERROR;
    }

    /* Select OLED. */
    HAL_GPIO_WritePin(
        OLED_CS_GPIO_Port,
        OLED_CS_Pin,
        GPIO_PIN_RESET
    );

    /* Data mode. */
    HAL_GPIO_WritePin(
        OLED_DC_GPIO_Port,
        OLED_DC_Pin,
        GPIO_PIN_SET
    );

    status = SPI_IF_Transmit(
        data,
        length,
        100U
    );

    /* Release OLED. */
    HAL_GPIO_WritePin(
        OLED_CS_GPIO_Port,
        OLED_CS_Pin,
        GPIO_PIN_SET
    );

    return status;
}

/**
 * @brief Initializes the OLED controller for a 128x64 display.
 *
 * The initialization sequence configures:
 * - display OFF during setup
 * - clock settings
 * - multiplex ratio for 64 rows
 * - display offset/start line
 * - internal charge pump
 * - horizontal memory addressing
 * - segment/COM orientation
 * - contrast
 * - pre-charge and VCOM settings
 * - normal display mode
 * - display ON
 */
HAL_StatusTypeDef OLED_Init(void)
{
    /*
     * Reset the OLED controller first so we start
     * from a known hardware state.
     */
    OLED_Reset();

    /*
     * Display OFF while configuring the controller.
     */
    if (OLED_WriteCommand(0xAE) != HAL_OK)
        return HAL_ERROR;

    /*
     * Set display clock divide ratio / oscillator frequency.
     */
    OLED_WriteCommand(0xD5);
    OLED_WriteCommand(0x80);

    /*
     * Multiplex ratio.
     *
     * 0x3F = 63 decimal.
     * Since counting starts at zero:
     *
     * 0..63 = 64 display rows.
     */
    OLED_WriteCommand(0xA8);
    OLED_WriteCommand(0x3F);

    /*
     * Display offset = 0.
     */
    OLED_WriteCommand(0xD3);
    OLED_WriteCommand(0x00);

    /*
     * Display start line = 0.
     */
    OLED_WriteCommand(0x40);

    /*
     * Enable the internal charge pump.
     */
    OLED_WriteCommand(0x8D);
    OLED_WriteCommand(0x14);

    /*
     * Select horizontal addressing mode.
     *
     * This makes it easy to write the framebuffer
     * sequentially from left to right and top to bottom.
     */
    OLED_WriteCommand(0x20);
    OLED_WriteCommand(0x00);

    /*
     * Segment remap.
     *
     * This controls horizontal display orientation.
     */
    OLED_WriteCommand(0xA1);

    /*
     * COM output scan direction.
     *
     * This controls vertical display orientation.
     */
    OLED_WriteCommand(0xC8);

    /*
     * Configure COM pins for a 128x64 panel.
     */
    OLED_WriteCommand(0xDA);
    OLED_WriteCommand(0x12);

    /*
     * Set contrast.
     */
    OLED_WriteCommand(0x81);
    OLED_WriteCommand(0x7F);

    /*
     * Set pre-charge period.
     */
    OLED_WriteCommand(0xD9);
    OLED_WriteCommand(0xF1);

    /*
     * Set VCOMH deselect level.
     */
    OLED_WriteCommand(0xDB);
    OLED_WriteCommand(0x40);

    /*
     * Entire display follows display RAM contents.
     */
    OLED_WriteCommand(0xA4);

    /*
     * Normal display:
     * 1 bits = illuminated pixels.
     */
    OLED_WriteCommand(0xA6);

    /*
     * Finally turn the display ON.
     */
    if (OLED_WriteCommand(0xAF) != HAL_OK)
        return HAL_ERROR;

    HAL_Delay(100);

    return HAL_OK;
}

/**
 * @brief Fills the entire 128x64 OLED display with ON pixels.
 *
 * A 128x64 monochrome OLED needs:
 *
 * 128 columns × 64 rows = 8192 pixels
 *
 * Each byte contains 8 vertical pixels:
 *
 * 8192 / 8 = 1024 bytes
 *
 * Sending 0xFF means every bit in every byte is 1,
 * therefore every pixel should illuminate.
 */
void OLED_FillTest(void)
{
    /*
     * One row of display RAM in our transfer buffer.
     *
     * 128 bytes × 8 pixels per byte.
     */
    uint8_t display_line[128];

    /*
     * Set every bit to 1.
     *
     * 0xFF binary = 11111111
     */
    memset(
        display_line,
        0xFF,
        sizeof(display_line)
    );

    /*
     * Define the column address range:
     *
     * 0 through 127.
     */
    OLED_WriteCommand(0x21);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x7F);

    /*
     * Define the page address range:
     *
     * Page 0 through Page 7.
     *
     * 8 pages × 8 pixels = 64 vertical pixels.
     */
    OLED_WriteCommand(0x22);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x07);

    /*
     * There are 8 pages.
     *
     * Send 128 bytes for each page:
     *
     * 8 × 128 = 1024 bytes total.
     */
    for (uint8_t page = 0; page < 8; page++)
    {
        OLED_WriteData(
            display_line,
            sizeof(display_line)
        );
    }
}


/*
 * Returns one column of a simple 5x7 font.
 *
 * Each character is 5 pixels wide.
 * Each byte represents one vertical column of 7 pixels.
 *
 * We only include the characters required by our display
 * for now. More characters can easily be added later.
 */
static uint8_t OLED_GetFontColumn(char c, uint8_t column)
{
    /*
     * Each row contains the five columns for one character.
     */

    static const uint8_t font_digits[10][5] =
    {
        {0x3E, 0x51, 0x49, 0x45, 0x3E}, /* 0 */
        {0x00, 0x42, 0x7F, 0x40, 0x00}, /* 1 */
        {0x42, 0x61, 0x51, 0x49, 0x46}, /* 2 */
        {0x21, 0x41, 0x45, 0x4B, 0x31}, /* 3 */
        {0x18, 0x14, 0x12, 0x7F, 0x10}, /* 4 */
        {0x27, 0x45, 0x45, 0x45, 0x39}, /* 5 */
        {0x3C, 0x4A, 0x49, 0x49, 0x30}, /* 6 */
        {0x01, 0x71, 0x09, 0x05, 0x03}, /* 7 */
        {0x36, 0x49, 0x49, 0x49, 0x36}, /* 8 */
        {0x06, 0x49, 0x49, 0x29, 0x1E}  /* 9 */
    };

    if ((c >= '0') && (c <= '9'))
    {
        return font_digits[c - '0'][column];
    }


 /* Capital letters and symbols used by our UI.
 *
 * Each case returns one 5-pixel-wide-by-7-pixel-tall glyph, one byte
 * at a time, selected by 'column' (0 to 4). Same encoding as the
 * digit table above: each byte is one vertical column, bit 0 at the
 * top row, bit 6 at the bottom row.
 *
 * Only the characters this project actually displays are defined
 * here. Add more cases the same way if new characters are needed.
 */
    switch (c)
    {
        case 'A':
        {
            static const uint8_t data[5] =
                {0x7E, 0x11, 0x11, 0x11, 0x7E};
            return data[column];
        }

        case 'C':
        {
            static const uint8_t data[5] =
                {0x3E, 0x41, 0x41, 0x41, 0x22};
            return data[column];
        }

        case 'E':
        {
            static const uint8_t data[5] =
                {0x7F, 0x49, 0x49, 0x49, 0x41};
            return data[column];
        }

        case 'H':
        {
            static const uint8_t data[5] =
                {0x7F, 0x08, 0x08, 0x08, 0x7F};
            return data[column];
        }

        case 'I':
        {
            static const uint8_t data[5] =
                {0x00, 0x41, 0x7F, 0x41, 0x00};
            return data[column];
        }

        case 'M':
        {
            static const uint8_t data[5] =
                {0x7F, 0x02, 0x0C, 0x02, 0x7F};
            return data[column];
        }

        case 'N':
        {
            static const uint8_t data[5] =
                {0x7F, 0x04, 0x08, 0x10, 0x7F};
            return data[column];
        }

        case 'O':
        {
            static const uint8_t data[5] =
                {0x3E, 0x41, 0x41, 0x41, 0x3E};
            return data[column];
        }

        case 'P':
        {
            static const uint8_t data[5] =
                {0x7F, 0x09, 0x09, 0x09, 0x06};
            return data[column];
        }

        case 'R':
        {
            static const uint8_t data[5] =
                {0x7F, 0x09, 0x19, 0x29, 0x46};
            return data[column];
        }

        case 'S':
        {
            static const uint8_t data[5] =
                {0x46, 0x49, 0x49, 0x49, 0x31};
            return data[column];
        }

        case 'T':
        {
            static const uint8_t data[5] =
                {0x01, 0x01, 0x7F, 0x01, 0x01};
            return data[column];
        }

        case 'U':
        {
            static const uint8_t data[5] =
                {0x3F, 0x40, 0x40, 0x40, 0x3F};
            return data[column];
        }

        case 'V':
        {
            static const uint8_t data[5] =
                {0x1F, 0x20, 0x40, 0x20, 0x1F};
            return data[column];
        }

        case ':':
        {
            static const uint8_t data[5] =
                {0x00, 0x36, 0x36, 0x00, 0x00};
            return data[column];
        }

        case '.':
        {
            static const uint8_t data[5] =
                {0x00, 0x60, 0x60, 0x00, 0x00};
            return data[column];
        }

        case '%':
        {
            static const uint8_t data[5] =
                {0x63, 0x13, 0x08, 0x64, 0x63};
            return data[column];
        }

        case '-':
        {
            static const uint8_t data[5] =
                {0x08, 0x08, 0x08, 0x08, 0x08};
            return data[column];
        }

        case ' ':
        default:
            return 0x00;
    }
}


/**
 * @brief Clears the local OLED framebuffer.
 *
 * This changes RAM inside the STM32.
 * It does NOT immediately transmit anything to the OLED.
 */
void OLED_Clear(void)
{
    memset(
        oled_buffer,
        0x00,
        sizeof(oled_buffer)
    );
}


/**
 * @brief Turns one pixel ON in the framebuffer.
 */
static void OLED_SetPixel(uint8_t x, uint8_t y)
{
    /*
     * Reject coordinates outside the 128x64 screen.
     */
    if ((x >= 128U) || (y >= 64U))
    {
        return;
    }

    /*
     * OLED memory is divided into 8-pixel-high pages.
     *
     * y / 8 tells us which page contains this pixel.
     *
     * y % 8 tells us which bit within that page.
     */
    uint16_t index =
        x + ((uint16_t)(y / 8U) * 128U);

    oled_buffer[index] |=
        (1U << (y % 8U));
}


/**
 * @brief Draws one 5x7 character into the framebuffer.
 */
static void OLED_DrawChar(
    uint8_t x,
    uint8_t y,
    char character)
{
    /*
     * Characters are five pixel columns wide.
     */
    for (uint8_t column = 0U; column < 5U; column++)
    {
        uint8_t column_data =
            OLED_GetFontColumn(character, column);

        /*
         * Each column contains seven vertical pixels.
         */
        for (uint8_t row = 0U; row < 7U; row++)
        {
            if (column_data & (1U << row))
            {
                OLED_SetPixel(
                    x + column,
                    y + row
                );
            }
        }
    }
}


/**
 * @brief Draws a null-terminated string.
 */
void OLED_DrawString(
    uint8_t x,
    uint8_t y,
    const char *text)
{
    if (text == NULL)
    {
        return;
    }

    while (*text != '\0')
    {
        OLED_DrawChar(
            x,
            y,
            *text
        );

        /*
         * Five pixels for the character
         * plus one pixel of spacing.
         */
        x += 6U;

        /*
         * Stop if another character would exceed
         * the display width.
         */
        if (x > 122U)
        {
            break;
        }

        text++;
    }
}


/**
 * @brief Transfers the complete framebuffer to the OLED.
 */


/**
 * brief Performs a hardware reset of the OLED controller.
 *
 * The RES pin is controlled by a normal GPIO.
 * Pulling RES low resets the OLED controller.
 * Bringing it high allows normal operation again.
 */

void OLED_UpdateScreen(void)
{
    /*
     * Select all 128 columns.
     */
    OLED_WriteCommand(0x21);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x7F);

    /*
     * Select all eight pages.
     */
    OLED_WriteCommand(0x22);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x07);

    /*
     * Send all 1024 framebuffer bytes.
     */
    OLED_WriteData(
        oled_buffer,
        sizeof(oled_buffer)
    );
}
