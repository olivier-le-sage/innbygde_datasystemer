/*
 * Node2.c
 *
 * Created: 10/13/2021 10:25:43 AM
 * Author : nilse
 */ 

#include <stddef.h>

#include "printf_stdarg.h"

#include "sam.h"
#include "uart.h"
#include "controls.h"
#include "CAN.h"

static void m_print_can_msg(const can_id_t * id, const can_data_t * data)
{
	if (data)
	{
		char data_str[8 * 2 + 7 + 1] = {0}; // max data bits * 2 + spaces + null byte
		if (data->len > 0)
		{	
			for (uint8_t i = 0; i < data->len; i++)
			{
				char *data_str_start = &data_str[2 * i + i];
				uart_snprintf(data_str_start, 3, "%02X", data->data[i]);
				data_str_start[2] = ' ';
			}
			data_str[3 * data->len - 1] = '\0';	
		}
		// For some reason printing the whole thing in one does not work
		// (maybe printf buffer size or something)
		uart_printf("[D] {ext: %u, id: %d, len: %u, data: [", id->extended, id->value, data->len);
		uart_printf("%s", &data_str[0]);
		uart_printf("] }\n");
	}
	else
	{
		uart_printf("[R] {ext: %u, id: %d }\n", id->extended, id->value);
	}
}

static void m_handle_can_rx(uint8_t rx_buf_no, const can_msg_rx_t *msg)
{
	uart_printf("RX: ");
	m_print_can_msg(&msg->id, msg->type == CAN_MSG_TYPE_DATA ? (joystick_direction_t)(&msg->data) : NULL);		
}

static void m_handle_can_tx(uint8_t tx_buf_no)
{
	uart_printf("TX complete.");
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
		// Same as the example in mcp2515 5.3, i.e. 125kHz CAN bus
		.bit = {
			.baudrate = 2000000,
			.sync_jump_len = 1,
			.prop_seg_len = 2,
			.phase_1_len = 7,
			.phase_2_len = 6
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
	
    /* Replace with your application code */
    while (1) 
    {
    }
}
