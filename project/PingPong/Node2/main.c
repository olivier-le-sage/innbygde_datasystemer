/*
 * Node2.c
 *
 * Created: 10/13/2021 10:25:43 AM
 * Author : nilse
 */ 


#include "printf_stdarg.h"

#include "sam.h"
#include "uart.h"


int main(void)
{
    /* Initialize the SAM system */
    SystemInit();

	uart_init();

	uart_printf("xyz\n");

    /* Replace with your application code */
    while (1) 
    {
    }
}
