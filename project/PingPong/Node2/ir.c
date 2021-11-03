/*
 * ir.c
 *
 * Created: 27.10.2021 17:52:36
 *  Author: oliviel
 */ 

#include "ir.h"
#include <sam3x8e.h>
#include <string.h>

/* Threshold at which the IR is considered blocked. Note: Max 12 bits. */
#define M_SAMPLE_THRESHOLD (0xFFF/5) // TODO: adjust

/* Number of consecutive samples required to trigger an interrupt */
#define M_SAMPLE_NUM_THRESHOLD 1 // cannot be zero

/* ADC channel used for the IR */
#define M_NUM_CHANNELS   (1)
#define M_IR_ADC_CHANNEL (0)

static uint16_t m_compe_count;

void ADC_Handler(void)
{
	// read out the status register
	volatile uint32_t interrupt_status = ADC->ADC_ISR;

	if (interrupt_status & ADC_ISR_COMPE)
	{
		m_compe_count++;
	}
}

void ir_adc_init(void)
{
	// reset internal state
	m_compe_count = 0;

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

	ADC->ADC_CWR = ADC_CWR_LOWTHRES(M_SAMPLE_THRESHOLD);

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

uint16_t ir_blocked_count_get(void)
{
	return m_compe_count;
}

ir_state_t ir_state_get(void)
{	
	if (m_compe_count)
	{
		return M_BLOCKED;
	}
	else
	{
		return M_NOT_BLOCKED;
	}
}