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

	// Test controls
	while(1)
	{
		joystick_direction_t first_direction;
		joystick_direction_t second_direction;
		get_joystick_dir(&first_direction, &second_direction);
		printf("Joystick: x-axis dir=%d, y-axis dir=%d\n", first_direction, second_direction);
		sliders_position_t sliders;
		get_sliders_pos(&sliders);
		printf("left slider=0x%x, right slider=0x%x\n", sliders.left_slider_pos, sliders.right_slider_pos);
		printf("\n");
	}

	char input;
	for (;;)
	{
		input = uart_fetch_by_force();
		//scanf("Type some number (not 'some number'): %d\n", &input);
		printf("You typed: %c\n", input);
	}
}
