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

	// Navigate user interface
	while(1)
	{
		joystick_direction_t x_dir;
		joystick_direction_t y_dir;
		get_joystick_dir(&x_dir, &y_dir);

		/*
		printf("Joystick: x-axis dir=%d, y-axis dir=%d\n", x_dir, y_dir);

		sliders_position_t sliders;
		get_sliders_pos(&sliders);
		printf("left slider=0x%x, right slider=0x%x\n", sliders.left_slider_pos, sliders.right_slider_pos);
		printf("\n");
		*/

		ui_cmd_t ui_cmd = UI_DO_NOTHING;

		if (x_dir == NEUTRAL && y_dir == UP)
		{
			ui_cmd = UI_SELECT_UP;
		}
		else if (x_dir == NEUTRAL && y_dir == DOWN)
		{
			ui_cmd = UI_SELECT_DOWN;
		}

		ui_issue_cmd(ui_cmd);
	}
}
