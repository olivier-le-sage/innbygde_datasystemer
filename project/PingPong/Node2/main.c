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
#define M_JOYSTICK_IMPACT_ON_MOTOR (500)
#define M_SLIDER_THRESHOLD_FOR_THRUST (0xFF/2) // 50%
#define M_SYSTICK_PERIOD_10MS (10)

// Pin used to control the solenoid
#define M_SOLENOID_PIN (1 << 3) // PORTD pin used to toggle solenoid // PIN28 // Arduino pin 27

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
	PIOD->PIO_CODR |= M_SOLENOID_PIN;
	for (int i = 0; i < 10; i++)
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

static void m_handle_can_rx(uint8_t rx_buf_no, const can_msg_rx_t *msg)
{
	static joystick_direction_t last_y_dir = UP;

	//uart_printf("RX: ");
	m_print_can_msg(&msg->id, msg->type == CAN_MSG_TYPE_DATA ? (&msg->data) : NULL);

	if (msg->id.value == CAN_JOYSTICK_MSG_ID && msg->data.len == 2)
	{
		/* Use the joystick direction values to adjust the servo position */
		joystick_direction_t x_dir = msg->data.data[0];
		if (x_dir == RIGHT)
		{
			//servo_position_goto(SERVO_POS_MIN);
			motor_pos_adjust(M_JOYSTICK_IMPACT_ON_MOTOR);
		}
		else if (x_dir == LEFT)
		{
			//servo_position_goto(SERVO_POS_MAX);
			motor_pos_adjust(-1 * M_JOYSTICK_IMPACT_ON_MOTOR);
		}
		else
		{
			servo_position_stop();
		}
		
		joystick_direction_t y_dir = msg->data.data[1];
		if (y_dir == UP && last_y_dir != UP)
		{
			m_toggle_solenoid_relay();
		}
		last_y_dir = y_dir;
	}
	else if (msg->id.value == CAN_SLIDER_MSG_ID && msg->data.len == 2)
	{
		uint8_t left_slider_pos = msg->data.data[1];
		servo_position_set(m_map_uint8_to_servo_position(left_slider_pos));
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

	debug_output_mck_on_pin();

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
	

    /* Replace with your application code */
    while (1)
    {
		/* Poll IR to get the user score. */
		// FIXME: should NOT call this frequently as it disables interrupts
		//if (ir_triggered_get_reset())
		{
			m_current_game_score++;
		}
		
		
		//uart_printf("< Current score: %d >\n", m_current_game_score);
    }
}
