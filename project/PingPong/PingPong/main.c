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

#define M_JOYSTICK_DATA_TXBUF_NO (0)

// initialize external memory mapping
// Sets the SRAM enable bit in the MCU control register
// and masks the top 4 bits of the addressing (reserved for JTAG)
#define ENABLE_SRAM() {MCUCR |= _BV(SRE); SFIOR |= _BV(XMM2);}

static joystick_direction_t m_x_dir;
static joystick_direction_t m_y_dir;
static sliders_position_t m_sliders;

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

// Fetch current joystick information and send it as as can message
static void m_send_joystick_direction_can_msg(void)
{
	can_data_t joystick_data;
	can_id_t joystick_data_id;
	uint8_t msg_data[2]; // one byte per direction
	
	get_joystick_dir(&m_x_dir, &m_y_dir);
	msg_data[0] = (uint8_t)m_x_dir;
	msg_data[1] = (uint8_t)m_y_dir;
	
	joystick_data.len = sizeof(msg_data);
	joystick_data.data = msg_data;

	joystick_data_id.value = 0xF; // TODO replace temp ID
	joystick_data_id.extended = false;
		
	// Use TX buffer 0 to send joystick direction
	can_data_send(M_JOYSTICK_DATA_TXBUF_NO, &joystick_data_id, &joystick_data);
}

// Handle received CAN messages
static void m_handle_can_rx(uint8_t rx_buf_no, const can_msg_rx_t *msg)
{
	printf("RX: ");
	m_print_can_msg(&msg->id, msg->type == CAN_MSG_TYPE_DATA ? &msg->data : NULL);		
}

static void m_handle_can_tx(uint8_t tx_buf_no)
{
	if (tx_buf_no == M_JOYSTICK_DATA_TXBUF_NO)
	{
		m_send_joystick_direction_can_msg();
	}
}

static uint8_t m_init_can()
{
	can_init_t init = {
		.rx_handler = m_handle_can_rx,
		.tx_handler = m_handle_can_tx,
		.buf = {
			.rx_buf_count = 1,
			.tx_buf_count = 1
		},
		.bit = {
			.baudrate = 1000000,
			.sync_jump_len = 1,
			.prop_seg_len = 1,
			.phase_1_len = 1,
			.phase_2_len = 1
		}
	};
	return can_init(&init);
}

int main(void)
{
	ENABLE_SRAM();
	assert(uart_init());
	assert(joystick_init());
	assert(ui_init());
	assert(m_init_can() == CAN_SUCCESS);

	m_send_joystick_direction_can_msg();

	// direct printf to the uart
	uart_config_streams();

	// Navigate user interface
	ui_cmd_t ui_cmd;
	

	while(1)
	{
		get_joystick_dir(&m_x_dir, &m_y_dir);
		//printf("Joystick: x-axis dir=%s, y-axis dir=%s\n", joystick_dir_to_str(m_x_dir), joystick_dir_to_str(m_y_dir));
		
		get_sliders_pos(&m_sliders);
		//printf("left slider=%d%%, right slider=%d%%\n", (m_sliders.left_slider_pos*100)/0xFF, (m_sliders.right_slider_pos*100)/0xFF);
		//printf("\n");

		_delay_ms(150); // the CPU is too fast

		ui_cmd = UI_DO_NOTHING;

		if (m_x_dir == NEUTRAL && m_y_dir == UP)
		{
			ui_cmd = UI_SELECT_DOWN;
		}
		else if (m_x_dir == NEUTRAL && m_y_dir == DOWN)
		{
			ui_cmd = UI_SELECT_UP;
		}
		else if (m_x_dir == RIGHT && m_y_dir == NEUTRAL)
		{
			ui_cmd = UI_ENTER_SUBMENU;
		}

		ui_issue_cmd(ui_cmd);
	}
}
