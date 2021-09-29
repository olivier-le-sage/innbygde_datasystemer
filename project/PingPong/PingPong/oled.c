/* Driver for the OLED display
 */

#include <stdio.h>
#include <stdarg.h>
#include "ext_peripherals.h"
#include "oled.h"
#include "oled_types.h"
#include "fonts.h"
#include "ping_pong.h"

static const uint8_t m_oled_init_routine[] =
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
	OLEDC_SET_MEM_ADDR_MODE_0, // Set page addressing mode 
	OLEDC_SET_MEM_ADDR_MODE_1(0x2), // 0x02,
	OLEDC_SET_VCOMH_DESELECT_LVL_0, // Set VCOMH deselect level 0.83 * Vcc
	OLEDC_SET_VCOMH_DESELECT_LVL_1(0x3), // 0x30,
	OLEDC_IREF_SELECTION_0, // 0xAD,
	OLEDC_IREF_SELECTION_1(0x0),
	OLEDC_ENTIRE_DISPLAY_ON(0x0), // 0xA4,
	OLEDC_SET_NORMAL_INVERSE_DISPLAY(0x0), // 0xA6,
	OLEDC_SET_DISPLAY_ON_OFF(0X1)
};

static int m_oled_printchar(char char_to_print, FILE *stream)
{
	assert((uint8_t)char_to_print <= NUM_LETTERS_IN_FONT);
	
	// TODO: Write oled_char_to_print to OLED data bus
	for (uint8_t i = 0; i < 5; i++)
	{
		EXT_OLED->DATA = font5[(uint8_t)char_to_print][i];
	}
	
	return 0;
}

/* Oled output stream */
static FILE m_oled_stream = FDEV_SETUP_STREAM(m_oled_printchar, NULL, _FDEV_SETUP_WRITE);

void oled_print(char *string)
{
	stdout = &m_oled_stream; // ensure stdout is pointing to the OLED and not RS232
	printf("%s", string);
}

bool oled_init(void)
{
	for (uint8_t i = 0; i < NUMELTS(m_oled_init_routine); i++)
	{
		EXT_OLED->CMD = m_oled_init_routine[i];			
	}
	return true;
}

void oled_reset(void)
{
	(void)oled_init();
}

void oled_home(void)
{
	// TODO
	/*
	//Set the cursor to the start of the screen
	*OLED_cmd = 0x21;
	*OLED_cmd = 0x00;
	*OLED_cmd = 0x7f;
	
	*OLED_cmd = 0x22;
	*OLED_cmd = 0x00;
	*OLED_cmd = 0x7;
	*/

}

void oled_goto_line(uint8_t line)
{
	// TODO
}

void oled_goto_column(uint8_t column)
{
	// TODO
}

void oled_clear_line(uint8_t line)
{
	// TODO
}

void oled_pos(uint8_t row, uint8_t column)
{
	// TODO
}