/*
 * joystick.c - Joystick driver
 *
 * Created: 15.09.2021 11:16:45
 *  Author: oliviel
 */ 

#include <avr/io.h>
#include "joystick.h"

#define M_ADC_ADDRESS     (0x1400)
#define M_MCU_RC_OSC_FREQ (8000000)
#define M_ADC_CLK_FREQ    (1000000) /* Allowed frequencies: 0.5MHz to 5MHz */
#define M_ADC_CLK_DIV     (0b001)   /* prescaler setting for clk/1 */
#define M_ADC_NUM_CH      (2)       /* The joystick has two voltage outputs */

// if a readout is above this value, the joystick is considered to be in that direction
// TODO: replace this temp value with one which is fine-tuned for a good user experience
#define M_ADC_DIRECTION_THRESHOLD (50) // %

// control pins for the ADC
#define M_ADC_WR (_BV(PD5))
#define M_ADC_RD (_BV(PD3))

static void m_run_sampling(uint8_t *p_channel_data_buffer)
{
	volatile uint8_t *ext_adc = (uint8_t *)M_ADC_ADDRESS;
	
	// initialize sampling by toggling WR, hopefully for long enough (TODO: Verify with scope)
	PORTD &= ~M_ADC_WR;
	PORTD |= M_ADC_WR;
	
	// Waste some time to allow conversion to finish
	// NB: conversion time is tconv = (9 x N x 2)/fclk
	// TODO: Don't do this
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	
	for (uint8_t i = 0; i < M_ADC_NUM_CH; i++)
	{
		// A readout is done by driving RD low twice
		PORTD &= ~M_ADC_RD;
		// The data can then be read directly from the ADC address space
		// The first RAM location read out is CH0, then CH1, and so on
		p_channel_data_buffer[i] = ext_adc[0];
		PORTD |= M_ADC_RD;
	}
}

static uint8_t m_convert_voltage_to_angle(uint8_t adc_sample)
{
	// TODO: Express relationship between joystick pot voltage and angle
	return 0;
}

void joystick_init(void)
{
	/* (I) Drive WR & RD high, since they are active-low. */
	DDRD |= M_ADC_RD | M_ADC_WR;
	
	/* (II) Setup PWM signal to clock the ADC, which will sample the joystick pot */
	OSCCAL = 0x7F; // auto-calibration setting: highest available frequency
	DDRD |= _BV(PD4); // PWM will be output on PD4
	
	/* Configure waveform generation with the following settings:
	 *   - phase-correct, frequency-correct pwm generation mode
	 *   - toggle OC3A on compare match
	 *   - clock divided by 1
	 */
	const uint8_t clock_source_mask = _BV(CS30) | _BV(CS31) | _BV(CS32);
	TCCR3A = _BV(WGM30) | _BV(COM3A0);
	TCCR3B = _BV(WGM33) | (clock_source_mask & M_ADC_CLK_DIV);
	
	// Output compare match must be at least one
	OCR3A = 1; // NB: We have found experimentally that this produces ~600 KHz.
	
	/* (III) */
	
	return true;
}

// DEBUG ONLY
void adc_test(uint8_t *p_sample_buffer)
{
	m_run_sampling(p_sample_buffer);
}

void get_joystick_pos(joystick_position_t *p_joystick_position_out)
{
	uint8_t adc_channels[M_ADC_NUM_CH];
	m_run_sampling((uint8_t*)adc_channels);
	
	p_joystick_position_out->x = m_convert_voltage_to_angle(adc_channels[0]);
	p_joystick_position_out->y = m_convert_voltage_to_angle(adc_channels[1]);
}

void get_joystick_dir(joystick_direction_t *p_first_dir_out, joystick_direction_t *p_second_dir_out)
{
	joystick_position_t joystick_analog_position;
	
	get_joystick_pos(&joystick_analog_position);
	
	/* Given the percentage values of the analog readout,
	 *   we establish the direction of the joystick using thresholds
	 */
	// TODO as it depends on the data format of the information coming from the ADC
	
	// return dummy values for now
	*p_first_dir_out = UP;
	*p_second_dir_out = LEFT;
}