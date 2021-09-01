/*
 * ping_pong.c
 *
 * Created: 2021-08-25 11:56:40 PM
 */

#include "ping_pong.h"
#include "rs232.h"

#define __NOP() {__asm__ __volatile__ ("nop");}

int main(void)
{
	assert(uart_init());

	for (;;)
	{
	    __NOP(); __NOP(); __NOP();
		printf("UART test.\n");
		__NOP(); __NOP(); __NOP();
	}

	uint32_t input;
	for (;;)
	{
		scanf("Type some number (not 'some number'): %d\n", &input);
		printf("You typed: %d\n", input);
	}
}
