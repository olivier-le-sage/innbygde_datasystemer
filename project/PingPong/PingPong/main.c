/*
 * ping_pong.c
 *
 * Created: 2021-08-25 11:56:40 PM
 */

#include "ping_pong.h"
#include "rs232.h"

#define __NOP() {__asm__ __volatile__ ("nop");}

// initialize external memory mapping
// Sets the SRAM enable bit in the MCU control register
#define ENABLE_SRAM() {MCUCR |= _BV(SRE);}

int main(void)
{
	ENABLE_SRAM();
	assert(uart_init());

	char input;
	for (;;)
	{
		input = uart_fetch_by_force();
		//scanf("Type some number (not 'some number'): %d\n", &input);
		printf("You typed: %c\n", input);
	}
}
