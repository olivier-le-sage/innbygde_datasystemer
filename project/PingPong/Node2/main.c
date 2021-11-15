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

/* TODO: Fine-tune these values for an enhanced user experience */
#define M_JOYSTICK_IMPACT_ON_MOTOR (100)
#define M_SLIDER_THRESHOLD_FOR_THRUST (0xFF/2) // 50%
#define M_SYSTICK_PERIOD_10MS (10)

/* Goals are registered when the IR beam is blocked. 1 block = 1 point */
static uint32_t m_current_game_score;

static void m_format_hex_byte(char * out, uint8_t value)
{
	uint8_t lsb = value & 0xF;
	uint8_t msb = (value >> 4) >> 0xF;

	out[0] = msb < 0xA ? '0' + msb : 'A' + (msb - 0xA);
	out[1] = lsb < 0xA ? '0' + lsb : 'A' + (lsb - 0xA);
}

static void m_toggle_solenoid_relay(void)
{
	PIOB->PIO_CODR = PIO_CODR_P1;
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	PIOB->PIO_SODR = PIO_SODR_P1;
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

static void m_handle_can_rx(uint8_t rx_buf_no, const can_msg_rx_t *msg)
{
	//uart_printf("RX: ");
	m_print_can_msg(&msg->id, msg->type == CAN_MSG_TYPE_DATA ? (&msg->data) : NULL);

	if (msg->id.value == CAN_JOYSTICK_MSG_ID && msg->data.len == 2)
	{
		/* Use the joystick direction values to adjust the servo position */
		joystick_direction_t x_dir = msg->data.data[0];
		if (x_dir == RIGHT)
		{
			servo_position_goto(SERVO_POS_MIN);
			motor_pos_adjust(M_JOYSTICK_IMPACT_ON_MOTOR);
		}
		else if (x_dir == LEFT)
		{
			servo_position_goto(SERVO_POS_MAX);
			motor_pos_adjust(-1 * M_JOYSTICK_IMPACT_ON_MOTOR);
		}
		else
		{
			servo_position_stop();
		}
	}
	else if (msg->id.value == CAN_SLIDER_MSG_ID && msg->data.len == 2)
	{
		if (msg->data.data[0] > M_SLIDER_THRESHOLD_FOR_THRUST)
		{
			m_toggle_solenoid_relay();
		}	
	}
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

static void debug_output_mck_on_pin(void)
{
	// Configure PCK1 to output MCK
	PMC->PMC_PCK[1] = PMC_PCK_CSS_MCK | PMC_PCK_PRES_CLK_1;
	// Enable PCK1 output
	PMC->PMC_SCER = PMC_SCER_PCK1;
}

static void m_systick_handler(void)
{
	motor_systick_handle();
}

static void m_systick_init(void)
{
	const systick_init_t params = {
		.cb = m_systick_handler,
		.period_10ms = M_SYSTICK_PERIOD_10MS
	};
	systick_init(&params);
}

int main(void)
{
    /* Initialize the SAM system */
    SystemInit();

	debug_output_mck_on_pin();

	m_score_reset();

	// Enable IO pin 33 for solenoid control
	// PIOB->PIO_PER = PIO_PER_P1;
	// PIOB->PIO_OER = PIO_OER_P1;

	// Disable watchdog timer (for now)
	WDT->WDT_MR = WDT_MR_WDDIS;

	uart_init();
	ir_adc_init();
	servo_init();
	m_can_init();
	motor_init();
	m_systick_init();

    /* Replace with your application code */
    while (1)
    {
		/* Poll IR to get the user score. */
		ir_state_t current_state = ir_state_get();

		if (current_state == IR_BLOCKED)
		{
			m_current_game_score++;
			ir_blocked_count_reset();
		}

		uart_printf("< Current score: %d >\n", m_current_game_score);
		
		//m_toggle_solenoid();
    }
}
