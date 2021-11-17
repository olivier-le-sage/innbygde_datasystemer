/*
 * ir.c
 *
 * Created: 27.10.2021 17:52:36
 *  Author: oliviel
 */

#include "ir.h"
#include <sam3x8e.h>
#include <string.h>
#include <stdbool.h>
#include "systick.h"

/* Threshold at which the IR is considered blocked. Note: Max 12 bits. */
#define M_SAMPLE_THRESHOLD (0xFFF/10)

/* Number of consecutive samples required to trigger an interrupt */
#define M_SAMPLE_NUM_THRESHOLD 1 // cannot be zero

#define M_EVENT_MIN_PERIOD_10MS 50

/* ADC channel used for the IR */
#define M_NUM_CHANNELS   (1)
#define M_IR_ADC_CHANNEL (0)

static volatile bool m_triggered;
static volatile uint32_t m_systick_cooldown_remaining;
static uint32_t m_systick_cooldown_period;

static void m_systick_handle(void)
{
	if (m_systick_cooldown_remaining)
	{
		m_systick_cooldown_remaining--;
	}
}

void ADC_Handler(void)
{
	// read out the status register
	volatile uint32_t interrupt_status = ADC->ADC_ISR;

	if ((interrupt_status & ADC_ISR_COMPE) && !m_systick_cooldown_remaining)
	{
		m_triggered = true;
		m_systick_cooldown_remaining = m_systick_cooldown_period;
	}
}

void ir_adc_init(void)
{
	// reset internal state
	m_triggered = false;
	m_systick_cooldown_remaining = 0;
	m_systick_cooldown_period = M_EVENT_MIN_PERIOD_10MS / systick_period_10ms_get();

	// Register systick callback
	systick_cb_register(m_systick_handle);

	PMC->PMC_PCR = PMC_PCR_PID(ID_ADC) |
				   PMC_PCR_CMD |
				   PMC_PCR_DIV_PERIPH_DIV_MCK |
				   PMC_PCR_EN;
    PMC->PMC_PCER1 |= (uint32_t) (32 - ID_ADC);

	// reset ADC
	ADC->ADC_CR |= ADC_CR_SWRST;

	// Disable ADC write protect
	ADC->ADC_WPMR = ADC_WPMR_WPKEY_PASSWD;

	// Mode Register settings
	ADC->ADC_MR = ADC_MR_TRGEN_DIS
				| ADC_MR_SLEEP_NORMAL
				| ADC_MR_FWUP_OFF
				| ADC_MR_PRESCAL(0xFF)
				| ADC_MR_STARTUP_SUT512
	            | ADC_MR_SETTLING_AST17
	            | ADC_MR_ANACH_NONE
	            | ADC_MR_TRACKTIM(0)
	            | ADC_MR_TRANSFER(1)
				| ADC_MR_USEQ_NUM_ORDER;

	// Configure Extended Mode Register to only generate interrupts when samples are below the threshold
	ADC->ADC_EMR = ADC_EMR_CMPMODE_LOW // Generate an interrupt when data is lower than the threshold
				 | ADC_EMR_CMPSEL(M_IR_ADC_CHANNEL)
				 | ADC_EMR_CMPFILTER(0);
				 // | ADC_EMR_CMPFILTER(M_SAMPLE_NUM_THRESHOLD);

	// ADC->ADC_CWR = ADC_CWR_LOWTHRES(M_SAMPLE_THRESHOLD);
	ADC->ADC_CWR = ADC_CWR_HIGHTHRES(0xFFF) | ADC_CWR_LOWTHRES(M_SAMPLE_THRESHOLD);

	// Enable channel
	ADC->ADC_CHDR = 0xFFFFFFFF & ~(1 << M_IR_ADC_CHANNEL); // disable all except the channel we want
	ADC->ADC_CHER |= (1 << M_IR_ADC_CHANNEL); // only enable the channel we want

	// Configure interrupts
	ADC->ADC_IER = ADC_IER_COMPE; // Comparison Event
	NVIC_EnableIRQ(ADC_IRQn);

	// Start ADC
	ADC->ADC_MR |= ADC_MR_FREERUN_ON; // free-running -> will never stop

	ADC->ADC_CR |= ADC_CR_START;
}

bool ir_triggered_get_reset(void)
{
	bool was_triggered;
	__disable_irq();
	was_triggered = m_triggered;
	m_triggered = false;
	__enable_irq();
	return was_triggered;
}

/*
uint32_t ir_blocked_count_get(void)
{
	return m_compe_count;
}

void ir_blocked_count_reset(void)
{
	m_compe_count = 0;
}

ir_state_t ir_state_get(void)
{
	if (m_compe_count)
	{
		return IR_BLOCKED;
	}
	else
	{
		return IR_NOT_BLOCKED;
	}
}
*/
