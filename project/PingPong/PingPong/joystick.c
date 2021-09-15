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
#define M_ADC_CLK_DIV     (0b010) /* prescaler setting for clk/8 */

static uint8_t m_read_word_from_adc(void)
{
	volatile uint8_t *ext_adc = (uint8_t *)M_ADC_ADDRESS;
	
	// TODO: RD and WR signals? BUSY? more?
	
	return ext_adc[0];
}

void joystick_init(void)
{
	OSCCAL = 0x7F; // auto-calibration setting: highest available frequency
	DDRD |= _BV(PD4); // PWM will be output on PD4
	
	/* Configure waveform generation with the following settings:
	 *   - phase-correct, frequency-correct pwm generation mode
	 *   - toggle OC3A on compare match
	 *   - clock divided by 8
	 */
	const uint8_t clock_source_mask = _BV(CS30) | _BV(CS31) | _BV(CS32);
	TCCR3A = _BV(WGM30) | _BV(COM3A0);
	TCCR3B = _BV(WGM33) | (clock_source_mask & M_ADC_CLK_DIV);
	
	// Setting to zero -> compare match each cycle -> PWM frequency = 2 * clk frequency.
	// Therefore we set the output compare to 1.
	OCR3A = 1;
}

void get_joystick_pos(joystick_position_t *p_joystick_direction_out)
{
	uint8_t adc_word = m_read_word_from_adc();
	(void)adc_word;
	
	// TODO: data format?
	
	// return dummy value for now
	p_joystick_direction_out->x = 0;
	p_joystick_direction_out->y = 0;
	p_joystick_direction_out->first_dir = UP;
	p_joystick_direction_out->second_dir = LEFT;
}

void get_joystick_dir(joystick_direction_t *p_first_dir_out, joystick_direction_t *p_second_dir_out)
{
	uint8_t adc_word = m_read_word_from_adc();
	(void)adc_word;

	// TODO: data format?
	
	// return dummy values for now
	*p_first_dir_out = UP;
	*p_second_dir_out = LEFT;
}