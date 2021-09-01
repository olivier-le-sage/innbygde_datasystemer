/*
 * ping_pong.c
 *
 * Created: 2021-08-25 11:56:40 PM
 */

#include <stdio.h>
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
}

