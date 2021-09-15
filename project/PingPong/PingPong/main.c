/*
 * ping_pong.c
 *
 * Created: 2021-08-25 11:56:40 PM
 */

#include "ping_pong.h"
#include "rs232.h"
#include "sram_test.h"
#include "joystick.h"

#define __NOP() {__asm__ __volatile__ ("nop");}

// initialize external memory mapping
// Sets the SRAM enable bit in the MCU control register
// and masks the top 4 bits of the addressing (reserved for JTAG)
#define ENABLE_SRAM() {MCUCR |= _BV(SRE); SFIOR |= _BV(XMM2);}

int main(void)
{
	ENABLE_SRAM();
	assert(uart_init());
	joystick_init();


	//SRAM_test();

	SRAM_test();

	char input;
	for (;;)
	{
		input = uart_fetch_by_force();
		//scanf("Type some number (not 'some number'): %d\n", &input);
		printf("You typed: %c\n", input);
	}
}
