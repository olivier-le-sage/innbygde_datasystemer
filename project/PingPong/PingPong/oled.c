/* Driver for the OLED display
 */

#include "oled.h"
#include "fonts.h"
#include "ping_pong.h"


void oled_print(char *output_char)
{
	// TODO
}

bool oled_init(void)
{
	/* Recommended initialization routine */
	char oled_init_routine[] =
	{
		0xAE, 0xA1, 0xDA, 0x12,
		0xC8, 0xA8, 0x3F, 0xD5,
		0x80, 0x81, 0x50, 0xD9,
		0x21, 0x20, 0x02, 0xDB,
		0x30, 0xAD, 0x00, 0xA4,
		0xA6, 0xAF
	};
	
	oled_print(oled_init_routine);

	return true; // Assume success for now
}

void oled_reset(void)
{
	// TODO
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