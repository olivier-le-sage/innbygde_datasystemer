/* Driver for the OLED display
 */

#include <stdio.h>
#include <stdarg.h>
#include "oled.h"
#include "fonts.h"
#include "ping_pong.h"

#define M_OLED_CMD_ADDR  ((char*)0x1000)
#define M_OLED_DATA_ADDR ((char*)0x1200)

static char oled_init_routine[] =
{
	0xAE, 0xA1, 0xDA, 0x12,
	0xC8, 0xA8, 0x3F, 0xD5,
	0x80, 0x81, 0x50, 0xD9,
	0x21, 0x20, 0x02, 0xDB,
	0x30, 0xAD, 0x00, 0xA4,
	0xA6, 0xAF
};



static int m_oled_printchar(char char_to_print, FILE *stream)
{
	assert((uint8_t)char_to_print <= NUM_LETTERS_IN_FONT);
	
	// TODO: Write oled_char_to_print to OLED data bus
	volatile uint8_t *ext_oled = (uint8_t *)M_OLED_DATA_ADDR;

	for (uint8_t i = 0; i < 5; i++)
	{
		ext_oled[i] = font5[(uint8_t)char_to_print][i];
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
	volatile uint8_t *ext_oled = (uint8_t *)M_OLED_CMD_ADDR;

	for (uint8_t i = 0; i < NUMELTS(oled_init_routine); i++)
	{
		ext_oled[0] = oled_init_routine[i];			
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