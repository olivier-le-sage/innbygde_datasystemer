/*
 * ping_pong.c
 *
 * Created: 2021-08-25 11:56:40 PM
 */

#include "ping_pong.h"
#include "rs232.h"
#include "sram_test.h"
#include "controls.h"
#include "ui.h"
#include "CAN.h"

// initialize external memory mapping
// Sets the SRAM enable bit in the MCU control register
// and masks the top 4 bits of the addressing (reserved for JTAG)
#define ENABLE_SRAM() {MCUCR |= _BV(SRE); SFIOR |= _BV(XMM2);}

int main(void)
{
	ENABLE_SRAM();
	assert(uart_init());
	assert(joystick_init());
	assert(ui_init());

	// direct printf to the uart
	uart_config_streams();

	// Navigate user interface
	ui_cmd_t ui_cmd;
	joystick_direction_t x_dir;
	joystick_direction_t y_dir;

	while(1)
	{
		get_joystick_dir(&x_dir, &y_dir);
		printf("Joystick: x-axis dir=%s, y-axis dir=%s\n", joystick_dir_to_str(x_dir), joystick_dir_to_str(y_dir));

		sliders_position_t sliders;
		get_sliders_pos(&sliders);
		printf("left slider=%d%%, right slider=%d%%\n", (sliders.left_slider_pos*100)/0xFF, (sliders.right_slider_pos*100)/0xFF);
		printf("\n");

		ui_cmd = UI_DO_NOTHING;

		if (x_dir == NEUTRAL && y_dir == UP)
		{
			ui_cmd = UI_SELECT_DOWN;
		}
		else if (x_dir == NEUTRAL && y_dir == DOWN)
		{
			ui_cmd = UI_SELECT_UP;
		}
		else if (x_dir == RIGHT && y_dir == NEUTRAL)
		{
			ui_cmd = UI_ENTER_SUBMENU;
		}

		ui_issue_cmd(ui_cmd);
	}
}
