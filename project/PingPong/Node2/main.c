/*
 * Node2.c
 *
 * Created: 10/13/2021 10:25:43 AM
 * Author : nilse
 */ 

#include "printf_stdarg.h"

#include "sam.h"
#include "uart.h"
#include "CAN.h"

static void m_handle_can_rx(uint8_t rx_buf_no, const can_msg_rx_t *msg)
{
	
}

static void m_handle_can_tx(uint8_t tx_buf_no)
{
	
}

static void m_can_init(void)
{
	can_init_t init = {
		.rx_handler = m_handle_can_rx,
		.tx_handler = m_handle_can_tx,
		.buf = {
			.rx_buf_count = 1,
			.tx_buf_count = 1
		},
		.bit = {
			.baudrate = 123, // FIXME
			.sync_jump_len = 1,
			.prop_seg_len = 1,
			.phase_1_len = 1,
			.phase_2_len = 1
		}
	};

	(void) can_init(&init);
}

int main(void)
{
    /* Initialize the SAM system */
    SystemInit();

	// Disable watchdog timer (for now)
	WDT->WDT_MR = WDT_MR_WDDIS;

	uart_init();
	m_can_init();

	uart_printf("xyz\n");

    /* Replace with your application code */
    while (1) 
    {
    }
}
