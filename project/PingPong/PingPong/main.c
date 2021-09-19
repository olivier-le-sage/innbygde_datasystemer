/*
 * ping_pong.c
 *
 * Created: 2021-08-25 11:56:40 PM
 */

#include "ping_pong.h"
#include "rs232.h"
#include "sram_test.h"
#include "controls.h"
#include "oled.h"
#include "ui.h"

// initialize external memory mapping
// Sets the SRAM enable bit in the MCU control register
// and masks the top 4 bits of the addressing (reserved for JTAG)
#define ENABLE_SRAM() {MCUCR |= _BV(SRE); SFIOR |= _BV(XMM2);}

int main(void)
{
	ENABLE_SRAM();
	assert(uart_init());
	assert(joystick_init());
	assert(oled_init());
	assert(ui_init());

	// Navigate user interface
	while(1)
	{
		joystick_direction_t joystick_x_axis;
		joystick_direction_t joystick_y_axis;
		get_joystick_dir(&joystick_x_axis, &joystick_y_axis);
		
		ui_cmd_t ui_cmd = UI_DO_NOTHING;
		
		if (joystick_x_axis == NEUTRAL && joystick_y_axis == UP)
		{
			ui_cmd = UI_SELECT_UP;
		}
		else if (joystick_x_axis == NEUTRAL && joystick_y_axis == DOWN)
		{
			ui_cmd = UI_SELECT_DOWN;
		}
		
		ui_issue_cmd(ui_cmd);
	}

	char input;
	for (;;)
	{
		input = uart_fetch_by_force();
		//scanf("Type some number (not 'some number'): %d\n", &input);
		printf("You typed: %c\n", input);
	}
}
