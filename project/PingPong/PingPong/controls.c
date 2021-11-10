/*
 * joystick.c - Joystick driver
 *
 * Created: 15.09.2021 11:16:45
 *  Author: oliviel
 */ 

#include <avr/io.h>
#include "ping_pong.h"
#include "controls.h"
#include <stdbool.h>
#include <util/delay.h>
#include "ext_peripherals.h"

#define CLOCK_SOURCE_MASK (_BV(CS30) | _BV(CS31) | _BV(CS32))

#define M_ADC_ADDRESS     (0x1400)
#define M_MCU_RC_OSC_FREQ (8000000)
#define M_ADC_CLK_FREQ    (1000000)  /* Allowed frequencies: 0.5MHz to 5MHz */
// FIXME: check if this should be lower to reduce power consumption
#define M_ADC_CLK_DIV     (0b001)    /* prescaler setting for clk/1 */
#define M_ADC_CLK_PIN     (_BV(PD4)) /* PWM will be output on PD4 */

/* Channel usage:
 *   0: joystick x-axis 
 *   1: joystick y-axis
 *   2: right slider
 *   3: left slider
 */
#define M_ADC_NUM_CH      (4)

#define M_R_BUTTON_PIN    (_BV(PB0)) /* input pin for the right touch-button */
#define M_L_BUTTON_PIN    (_BV(PB1)) /* input pin for the left touch-button */

// if a readout is above this value, the joystick is considered to be in that direction
// TODO: replace this temp value with one which is fine-tuned for a good user experience
#define M_JOYSTICK_X_AXIS_NEUTRAL  (0xA0)
#define M_JOYSTICK_Y_AXIS_NEUTRAL  (0x9E)
#define M_JOYSTICK_DIR_THRESHOLD   (0x40) // relative difference to neutral required to exit the "dead zone"
#define M_JOYSTICK_LEFT_THRESHOLD  (M_JOYSTICK_X_AXIS_NEUTRAL - M_JOYSTICK_DIR_THRESHOLD)
#define M_JOYSTICK_RIGHT_THRESHOLD (M_JOYSTICK_X_AXIS_NEUTRAL + M_JOYSTICK_DIR_THRESHOLD)
#define M_JOYSTICK_DOWN_THRESHOLD  (M_JOYSTICK_Y_AXIS_NEUTRAL - M_JOYSTICK_DIR_THRESHOLD)
#define M_JOYSTICK_UP_THRESHOLD    (M_JOYSTICK_Y_AXIS_NEUTRAL + M_JOYSTICK_DIR_THRESHOLD)

static void m_run_sampling(uint8_t *p_channel_data_buffer)
{
	// toggle WR by writing to the ADC's address space
	*EXT_ADC = 0;

	// Waste some time to allow conversion to finish
	// NB: conversion time is tconv = (9 x N x 2)/fclk
	_delay_us(50);
	
	for (uint8_t i = 0; i < M_ADC_NUM_CH; i++)
	{
		// The data can be read directly from the ADC address space
		// The first RAM location read out is CH0, then CH1, and so on
		p_channel_data_buffer[i] = *EXT_ADC;
	}
}

static uint8_t m_convert_voltage_to_angle(uint8_t adc_sample)
{
	/* Joystick voltage is 0 when on one side and 5v on the other.
	   Therefore there is no real need to do anything with the value returned from the ADC*/
	return adc_sample;
}

bool joystick_init(void)
{
	/* (I) Setup PWM signal to clock the ADC, which will sample the joystick pot */
	OSCCAL = 0x7F; // auto-calibration setting: highest available frequency
	DDRD |= M_ADC_CLK_PIN;
	
	/* Configure waveform generation with the following settings:
	 *   - phase-correct, frequency-correct pwm generation mode
	 *   - toggle OC3A on compare match
	 *   - clock divided by 1
	 */
	TCCR3A = _BV(WGM30) | _BV(COM3A0);
	TCCR3B = _BV(WGM33) | (CLOCK_SOURCE_MASK & M_ADC_CLK_DIV);
	
	// Output compare match must be at least one
	OCR3A = 1; // NB: We have found experimentally that this produces ~600 KHz.
	
	/* (II) Setup ports for button inputs */
	DDRB &= ~M_R_BUTTON_PIN;
	DDRB &= ~M_L_BUTTON_PIN;
	
	return true;
}

void get_joystick_pos(joystick_position_t *p_joystick_position_out)
{
	uint8_t adc_channels[M_ADC_NUM_CH];
	m_run_sampling((uint8_t*)adc_channels);
	
	p_joystick_position_out->x = m_convert_voltage_to_angle(adc_channels[0]);
	p_joystick_position_out->y = m_convert_voltage_to_angle(adc_channels[1]);
}

// FIXME: should sample joystick/sliders all at once to reduce time/energy
void get_joystick_dir(joystick_direction_t *p_first_dir_out, joystick_direction_t *p_second_dir_out)
{
	joystick_position_t joystick_analog_position;
	get_joystick_pos(&joystick_analog_position);
	
	/* Given the percentage values of the analog readout,
	 *   we establish the direction of the joystick using thresholds
	 */
	
	if (joystick_analog_position.x <= M_JOYSTICK_LEFT_THRESHOLD)
	{
		*p_first_dir_out = LEFT;
	}
	else if (joystick_analog_position.x >= M_JOYSTICK_RIGHT_THRESHOLD)
	{
		*p_first_dir_out = RIGHT;
	}
	else
	{
		*p_first_dir_out = NEUTRAL;
	}
	
	if (joystick_analog_position.y <= M_JOYSTICK_DOWN_THRESHOLD)
	{
		*p_second_dir_out = DOWN;
	}
	else if (joystick_analog_position.y >= M_JOYSTICK_UP_THRESHOLD)
	{
		*p_second_dir_out = UP;
	}
	else
	{
		*p_second_dir_out = NEUTRAL;
	}
}

void get_sliders_pos(sliders_position_t *p_slider_position_out)
{
	uint8_t adc_channels[M_ADC_NUM_CH];
	m_run_sampling((uint8_t*)adc_channels);
	
	p_slider_position_out->right_slider_pos = adc_channels[2];
	p_slider_position_out->left_slider_pos  = adc_channels[3];
}

void joystick_sleep(void)
{
	// Turn off clock to the ADC
	TCCR3B &= CLOCK_SOURCE_MASK;
}

void joystick_wake(void)
{
	// Turn on clock to the ADC
	TCCR3B |= (CLOCK_SOURCE_MASK & M_ADC_CLK_DIV);
}