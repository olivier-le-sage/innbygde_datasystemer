/*
 * ping_pong.c
 *
 * Created: 2021-08-25 11:56:40 PM
 */

#include "ping_pong.h"
#include "rs232.h"

int main(void)
{
	assert(uart_init());
	
	printf("UART test.\n");

	for (;;);
}

