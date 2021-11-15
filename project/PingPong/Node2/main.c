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
#include "servo.h"
#include "ir.h"
#include "CAN.h"
#include "motor.h"
#include "systick.h"
#include "joystick_state.h"

extern joystick_direction_t joystick_x_dir;
extern joystick_direction_t joystick_y_dir;

/* TODO: Fine-tune these values for an enhanced user experience */
#define M_SLIDER_THRESHOLD_FOR_THRUST (0xFF/2) // 50%
#define M_SYSTICK_PERIOD_10MS (10)

// Pin used to control the solenoid
#define M_SOLENOID_PIN (1 << 3) // PORTD pin used to toggle solenoid // PIN28 // Arduino pin 27
#define M_SCORE_TXBUF_NO (0)

/* Goals are registered when the IR beam is blocked. 1 block = 1 point */
static uint32_t m_current_game_score;
static volatile bool m_send_game_score;
static volatile bool m_send_game_score_in_progress;

static inline bool atomic_set(volatile bool * flag)
{
	bool prev_value;

	__disable_irq();
	prev_value = *flag;
	*flag = true;
	__enable_irq();

	return prev_value;
}

static void m_format_hex_byte(char * out, uint8_t value)
{
	uint8_t lsb = value & 0xF;
	uint8_t msb = (value >> 4) >> 0xF;

	out[0] = msb < 0xA ? '0' + msb : 'A' + (msb - 0xA);
	out[1] = lsb < 0xA ? '0' + lsb : 'A' + (lsb - 0xA);
}

static void m_toggle_solenoid_relay(void)
{
	PIOD->PIO_CODR |= M_SOLENOID_PIN;
	for (int i = 0; i < 100000; i++)
	{
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
	}

	PIOD->PIO_SODR |= M_SOLENOID_PIN;
}

static void m_print_can_msg(const can_id_t * id, const can_data_t * data)
{
	if (data)
	{
		if (id->value == CAN_JOYSTICK_MSG_ID && data->len == 2)
		{
			/* Message contains joystick direction, interpret it as such */
			/*
			uart_printf("[Joystick Direction] {%s, %s}\n",
						joystick_dir_to_str((joystick_direction_t)data->data[0]),
						joystick_dir_to_str((joystick_direction_t)data->data[1]));
		    */
		}
		else if (id->value == CAN_SLIDER_MSG_ID && data->len == 2)
		{
			/* Message contains slider position information, interpret it as such. */
			/*
			uart_printf("[Slider Position] {%d%%, %d%%}\n",
					    ((uint32_t)data->data[0]*100)/0xFF,
					    ((uint32_t)data->data[1]*100)/0xFF);
			*/
		}
		else
		{
			char data_str[8 * 2 + 7 + 1] = {0}; // max data bits * 2 + spaces + null byte
			if (data->len > 0)
			{
				for (uint8_t i = 0; i < data->len; i++)
				{
					char *data_str_start = &data_str[2 * i + i];
					m_format_hex_byte(data_str_start, data->data[i]);
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
	}
	else
	{
		uart_printf("[R] {ext: %u, id: %d }\n", id->extended, id->value);
	}
}

static inline uint16_t m_map_uint8_to_servo_position(uint8_t value)
{
	return (uint16_t) (((uint32_t) (SERVO_POS_MAX - SERVO_POS_MIN) * (uint32_t) value) / (uint32_t) UINT8_MAX);
}

#define M_SERVO_VALUE_HISTORY_LEN 2

static void m_update_servo_position(uint8_t new_value)
{
	static uint8_t last_values[M_SERVO_VALUE_HISTORY_LEN] = { UINT8_MAX / 2 };
	static uint32_t value_pos;

	uint32_t value_sum = 0;

	last_values[value_pos] = new_value;
	value_pos = (value_pos + 1) % (M_SERVO_VALUE_HISTORY_LEN);

	for (uint32_t i = 0; i < M_SERVO_VALUE_HISTORY_LEN; i++)
	{
		value_sum += last_values[i];
	}

	value_sum /= M_SERVO_VALUE_HISTORY_LEN;
	servo_position_set(m_map_uint8_to_servo_position((uint8_t) value_sum));
}

static void m_handle_can_rx(uint8_t rx_buf_no, const can_msg_rx_t *msg)
{
	static joystick_direction_t last_y_dir = UP;

	//uart_printf("RX: ");
	m_print_can_msg(&msg->id, msg->type == CAN_MSG_TYPE_DATA ? (&msg->data) : NULL);

	if (msg->id.value == CAN_JOYSTICK_MSG_ID && msg->data.len == 2)
	{
		/* Save joystick position. These values will be used elsewhere */
		joystick_x_dir = msg->data.data[0];
		joystick_y_dir = msg->data.data[1];
		
		if (joystick_y_dir == UP && last_y_dir == UP)
		{
			PIOD->PIO_CODR |= M_SOLENOID_PIN;
		}
		else
		{
			PIOD->PIO_SODR |= M_SOLENOID_PIN;
		}
		last_y_dir = joystick_y_dir;
	}
	else if (msg->id.value == CAN_SLIDER_MSG_ID && msg->data.len == 2)
	{
		uint8_t left_slider_pos = msg->data.data[1];
		m_update_servo_position(UINT8_MAX - left_slider_pos);
	}
}

static void m_handle_can_tx(uint8_t tx_buf_no)
{
	if (tx_buf_no == M_SCORE_TXBUF_NO)
	{
		m_send_game_score_in_progress = false;
	}
}

static void m_can_init(void)
{
	can_init_t init = {
		.rx_handler = m_handle_can_rx,
		.tx_handler = m_handle_can_tx,
		.buf = {
			.rx_buf_count = 2,
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

static void m_score_reset(void)
{
	m_current_game_score = 0;
}

static bool m_score_send(void)
{
	static can_id_t id = { .extended = false, .value = CAN_SCORE_MSG_ID };
	static uint32_t score;
	static can_data_t data = { .data = (const uint8_t *) &score, .len = sizeof(score) };

	if (!atomic_set(&m_send_game_score_in_progress))
	{
		return false;
	}

	score = m_current_game_score;

	uint8_t rc = can_data_send(M_SCORE_TXBUF_NO, &id, &data);
	if (rc != CAN_SUCCESS)
	{
		m_send_game_score_in_progress = false;
		return false;
	}

	return true;
}

static void m_sleep(void)
{
	(void) can_sleep();
	power_sleep();
	(void) can_wake();
}

static void m_systick_init(void)
{
	const systick_init_t params = {
		.period_10ms = M_SYSTICK_PERIOD_10MS
	};
	systick_init(&params);
}

int main(void)
{
    /* Initialize the SAM system */
    SystemInit();

	m_score_reset();

	// Disable watchdog timer (for now)
	WDT->WDT_MR = WDT_MR_WDDIS;

	m_systick_init();

	uart_init();
	ir_adc_init();
	servo_init();
	m_can_init();
	motor_init();

	// Enable IO pin  for solenoid control
	PIOD->PIO_PER |= M_SOLENOID_PIN;
	PIOD->PIO_OER |= M_SOLENOID_PIN;
	PIOD->PIO_PDR &= ~M_SOLENOID_PIN;
	PIOD->PIO_ODR &= ~M_SOLENOID_PIN;

	systick_enable();

	m_send_game_score = true;

    while (1)
    {
		/* Poll IR to get the user score. */
		if (ir_triggered_get_reset())
		{
			m_current_game_score++;
			m_send_game_score = true;
		}

		if (m_send_game_score)
		{
			m_send_game_score = !m_score_send();
		}
		else
		{
			m_sleep();
		}
    }
}
