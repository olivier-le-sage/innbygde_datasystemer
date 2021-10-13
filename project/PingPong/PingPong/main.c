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
#include "CAN.h"
#include <util/delay.h>


// initialize external memory mapping
// Sets the SRAM enable bit in the MCU control register
// and masks the top 4 bits of the addressing (reserved for JTAG)
#define ENABLE_SRAM() {MCUCR |= _BV(SRE); SFIOR |= _BV(XMM2);}


static void m_print_can_msg(const can_id_t * id, const can_data_t * data)
{
	assert(id);

	if (data)
	{
		char data_str[8 * 2 + 7 + 1] = {0}; // max data bits * 2 + spaces + null byte
		if (data->len > 0)
		{	
			for (uint8_t i = 0; i < data->len; i++)
			{
				char *data_str_start = &data_str[2 * i + i];
				snprintf(data_str_start, 3, "%02X", data->data[i]);
				data_str_start[2] = ' ';
			}
			data_str[3 * data->len - 1] = '\0';	
		}
		// For some reason printing the whole thing in one does not work
		// (maybe printf buffer size or something)
		printf("[D] {ext: %u, id: %d, len: %u, data: [", (uint32_t)id->extended, (int)id->value, (uint32_t)data->len);
		printf("%s", &data_str[0]);
		printf("] }\n");
	}
	else
	{
		printf("[R] {ext: %u, id: %d }\n", (uint32_t)id->extended, (int)id->value);
	}
}

// Handle received CAN messages
static void m_handle_can_rx(uint8_t rx_buf_no, const can_msg_rx_t *msg)
{
	printf("RX: ");
	m_print_can_msg(&msg->id, msg->type == CAN_MSG_TYPE_DATA ? &msg->data : NULL);		
}

static void m_handle_can_tx(uint8_t tx_buf_no)
{
}

static bool m_init_can()
{
	can_init_t init = {
		.rx_handler = m_handle_can_rx,
		.tx_handler = m_handle_can_tx
	};
	return can_init(&init);
}

int main(void)
{
	ENABLE_SRAM();
	assert(uart_init());
	assert(joystick_init());
	assert(ui_init());
	assert(m_init_can());

	// direct printf to the uart
	uart_config_streams();

	// Navigate user interface
	ui_cmd_t ui_cmd;
	joystick_direction_t x_dir;
	joystick_direction_t y_dir;

	while(1)
	{
		get_joystick_dir(&x_dir, &y_dir);
		printf("Joystick: x-axis dir=%s, y-axis dir=%s\n", joystick_dir_to_str(x_dir), joystick_dir_to_str(y_dir));

		sliders_position_t sliders;
		get_sliders_pos(&sliders);
		printf("left slider=%d%%, right slider=%d%%\n", (sliders.left_slider_pos*100)/0xFF, (sliders.right_slider_pos*100)/0xFF);
		printf("\n");

		can_data_t joystick_data;
		uint8_t msg_data[2]; // one byte per direction
		msg_data[0] = (uint8_t)x_dir;
		msg_data[1] = (uint8_t)y_dir;
		joystick_data.len = sizeof(msg_data);
		joystick_data.data = msg_data;

		can_id_t joystick_data_id;
		joystick_data_id.value = 0xF; // temp value, replace with some value
		joystick_data_id.extended = false;
		
		// Use TX buffer 0 to send joystick direction
		can_data_send(0, &joystick_data_id, &joystick_data);

		ui_cmd = UI_DO_NOTHING;

		if (x_dir == NEUTRAL && y_dir == UP)
		{
			ui_cmd = UI_SELECT_DOWN;
		}
		else if (x_dir == NEUTRAL && y_dir == DOWN)
		{
			ui_cmd = UI_SELECT_UP;
		}
		else if (x_dir == RIGHT && y_dir == NEUTRAL)
		{
			ui_cmd = UI_ENTER_SUBMENU;
		}

		ui_issue_cmd(ui_cmd);
	}
}
