/* Driver for the OLED display
 */

#include <stdio.h>
#include <stdarg.h>
#include "ext_peripherals.h"
#include "oled.h"
#include "oled_types.h"
#include "fonts.h"
#include "ping_pong.h"


/*
static char oled_init_routine[] =
{
    0xAE,
    0xA1,
    0xDA,
    0x12,
    0xC8,
    0xA8,
    0x3F,
    0xD5,
    0x80,
    0x81,
    0x50,
    0xD9,
    0x21,
    0x20,
    0x02,
    0xDB,
    0x30,
    0xAD,
    0x00,
    0xA4,
    0xA6,
    0xAF
};
*/

static char m_oled_init_routine[] =
{
    OLEDC_SET_DISPLAY_ON_OFF(0x0), // Turn display off
    OLEDC_SET_SEGMENT_REMAP(0x1), // Map column addr 127 to SEG0
    OLEDC_SET_COM_PIN_HW_CFG_0, //
    OLEDC_SET_COM_PIN_HW_CFG_1(0x1), // Set alternative COM pin config
    OLEDC_SET_COM_OUT_SCAN_DIR(0x1), // Set COM output scan direction from COM0-COM[N-1]
    OLEDC_SET_MUX_RATIO_0, // Set multiplex ratio to 63
    OLEDC_SET_MUX_RATIO_1(0x3F),
    OLEDC_SET_DISPLAY_CLOCK_DIV_0, // Set display clock divisor
    OLEDC_SET_DISPLAY_CLOCK_DIV_1(0x80),
    OLEDC_SET_CONTRAST_CONTROL_0, // 0x81,
    OLEDC_SET_CONTRAST_CONTROL_1(0x50), // 0x50,
    OLEDC_SET_PRE_CHARGE_PERIOD_0, // 0xD9,
    OLEDC_SET_PRE_CHARGE_PERIOD_1(0x21),
    OLEDC_SET_MEM_ADDR_MODE_0,
    OLEDC_SET_MEM_ADDR_MODE_1(0x2), // Set page addressing mode
    OLEDC_SET_VCOMH_DESELECT_LVL_0, // Set VCOMH deselect level 0.83 * Vcc
    OLEDC_SET_VCOMH_DESELECT_LVL_1(0x3), // 0x30,
    OLEDC_IREF_SELECTION_0, // 0xAD,
    OLEDC_IREF_SELECTION_1(0x0),
    OLEDC_ENTIRE_DISPLAY_ON(0x0), // 0xA4,
    OLEDC_SET_NORMAL_INVERSE_DISPLAY(0x0), // 0xA6,
    OLEDC_SET_DISPLAY_ON_OFF(0X1)
};

static uint8_t m_current_row;
static uint8_t m_current_col;

static int m_oled_printchar(char char_to_print, FILE *stream)
{
    assert((uint8_t)char_to_print <= NUM_LETTERS_IN_FONT);

	const uint8_t font_size = NUMELTS(font4[0]);

    for (uint8_t i = 0; i < font_size; i++)
    {
		volatile uint8_t char_to_write = pgm_read_word(&font4[(uint8_t)char_to_print-32][i]);
        EXT_OLED->DATA = char_to_write;
		m_current_col = (m_current_col + font_size) % 128;
    }

    return 0;
}

static int m_oled_inv_printchar(char char_to_print, FILE *stream)
{
	assert((uint8_t)char_to_print <= NUM_LETTERS_IN_FONT);

	const uint8_t font_size = NUMELTS(font4[0]);

	for (uint8_t i = 0; i < font_size; i++)
	{
		volatile uint8_t char_to_write = ~pgm_read_word(&font4[(uint8_t)char_to_print-32][i]);
		EXT_OLED->DATA = char_to_write;
		m_current_col = (m_current_col + font_size) % 128;
	}

	return 0;
}

/* Oled output streams */
static FILE m_oled_stream = FDEV_SETUP_STREAM(m_oled_printchar, NULL, _FDEV_SETUP_WRITE);
static FILE m_oled_inv_stream = FDEV_SETUP_STREAM(m_oled_inv_printchar, NULL, _FDEV_SETUP_WRITE);

void oled_printf(const char *string, bool inv, ...)
{
	va_list args;
	va_start(args, inv);

	if (inv)
	{
		vfprintf(&m_oled_inv_stream, string, args);
	}
	else
	{
		vfprintf(&m_oled_stream, string, args);
	}

	va_end(args);
}



bool oled_init(void)
{
    for (uint8_t i = 0; i < NUMELTS(m_oled_init_routine); i++)
    {
        EXT_OLED->CMD = m_oled_init_routine[i];
    }

    for (uint8_t line = 0; line < 8; line++)
    {
        oled_clear_line(line);
    }

    oled_home();

    return true;
}

void oled_reset(void)
{
    for (uint8_t line = 0; line < 8; line++)
    {
        oled_clear_line(line);
    }

    oled_home();
}

void oled_home(void)
{
    oled_pos(0, 0);
}

void oled_goto_line(uint8_t line)
{
    assert(line < 8);

    EXT_OLED->CMD = 0xb0 | line;
	m_current_row = line;
	
	oled_goto_column(0);
}

void oled_goto_column(uint8_t column)
{
    assert(column < 128);

    // lower 4 bits
    EXT_OLED->CMD = 0x00 | (column & 0x0F);

    // upper 4 bits
    EXT_OLED->CMD = 0x10 | (column >> 4);

	m_current_col = column;
}

void oled_clear_line(uint8_t line)
{
    oled_goto_line(line);

    for (uint8_t i = 0; i < 128; i++)
    {
        EXT_OLED->DATA = 0;
    }
}

void oled_pos(uint8_t row, uint8_t column)
{
    /* Treat each page as a row
     * That gives us 8 rows in total */
    oled_goto_line(row);

    /* Column selection */
    oled_goto_column(column);
}
