#include "servo.h"

#include <sam3x8e.h>

// Minimum allowed duty cycle in PWM ticks (=0.9ms)
#define SERVO_MIN_STEPS 90
// Maximum allowed duty cycle in PWM ticks (=2.1ms)
#define SERVO_MAX_STEPS 210
// Duty cycle for neutral servo position (apx. 1.5ms)
#define SERVO_NEUTRAL_STEPS ((SERVO_MAX_STEPS + SERVO_MIN_STEPS) / 2)
// Number of servo positions
#define SERVO_STEP_COUNT (SERVO_MAX_STEPS - SERVO_MIN_STEPS)
// Neutral servo position ("absolute")
#define SERVO_NEUTRAL_POS (SERVO_NEUTRAL_STEPS - SERVO_MIN_STEPS)

#define SERVO_TARGET_POS_INVALID INT16_MAX

// Period for the PWM used for the servo
#define SERVO_PWM_PERIOD 2000

#define MCK_8_FACTOR_FOR_TICK 105

#define TC_RC_VALUE (SERVO_PWM_PERIOD * MCK_8_FACTOR_FOR_TICK)
#define TC_RA_VALUE(_v) (TC_RC_VALUE - (((uint32_t) (_v)) * MCK_8_FACTOR_FOR_TICK))

static volatile int16_t m_current_servo_position;
static volatile int16_t m_target_servo_position = SERVO_TARGET_POS_INVALID;

void TC0_Handler(void)
{
    uint32_t status = TC0->TC_CHANNEL[0].TC_SR;
    if (status & TC_SR_CPCS &&
        m_target_servo_position != SERVO_TARGET_POS_INVALID)
    {
        if (m_current_servo_position < m_target_servo_position)
        {
            m_current_servo_position++;
            TC0->TC_CHANNEL[0].TC_RA = TC_RA_VALUE(m_current_servo_position + SERVO_MIN_STEPS);
        }
        else if (m_current_servo_position > m_target_servo_position)
        {
            m_current_servo_position--;
            TC0->TC_CHANNEL[0].TC_RA = TC_RA_VALUE(m_current_servo_position + SERVO_MIN_STEPS);
        }
        else
        {
            m_target_servo_position = SERVO_TARGET_POS_INVALID;
            TC0->TC_CHANNEL[0].TC_IDR = TC_IDR_CPCS;
        }
    }
}

void servo_init(void)
{
    // Enable clock
    PMC->PMC_PCR = PMC_PCR_PID(ID_TC0) |
                   PMC_PCR_CMD |
                   PMC_PCR_DIV_PERIPH_DIV_MCK |
                   PMC_PCR_EN;
    PMC->PMC_PCER0 |= (uint32_t) (1 << ID_TC0);

	TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKDIS;

    // Configure output pin for TC0 TIOA0
    PIOB->PIO_IDR = PIO_PB25B_TIOA0; // Disable interrupt on pin
    PIOB->PIO_ABSR |= PIO_PB25B_TIOA0; // Assign pin to peripheral B
    PIOB->PIO_PDR = PIO_PB25B_TIOA0;  // Let peripheral control pin

    TC0->TC_CHANNEL[0].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK2 |
                   TC_CMR_WAVE |
                   TC_CMR_WAVSEL_UP_RC |
                   TC_CMR_ACPA_SET |
                   TC_CMR_ACPC_CLEAR;

    // Reset counter on this value
    TC0->TC_CHANNEL[0].TC_RC = SERVO_PWM_PERIOD * MCK_8_FACTOR_FOR_TICK;
    // Set neutral position
    TC0->TC_CHANNEL[0].TC_RA = TC_RA_VALUE(SERVO_NEUTRAL_STEPS);

    TC0->TC_CHANNEL[0].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;

    NVIC_EnableIRQ(TC0_IRQn);

    m_current_servo_position = SERVO_NEUTRAL_POS;
}

void servo_position_adjust(int16_t delta)
{
    if ((m_current_servo_position + SERVO_MIN_STEPS + delta >= SERVO_MIN_STEPS)
        && ( m_current_servo_position + SERVO_MIN_STEPS + delta <= SERVO_MAX_STEPS))
    {
        m_current_servo_position += delta;
        TC0->TC_CHANNEL[0].TC_RA = TC_RA_VALUE((uint16_t)m_current_servo_position + SERVO_MIN_STEPS);
    }
}

void servo_position_goto(uint16_t position)
{
    if (position >= SERVO_STEP_COUNT)
    {
        return;
    }

    NVIC_DisableIRQ(TC0_IRQn);
    m_target_servo_position = position;
    TC0->TC_CHANNEL[0].TC_IER = TC_IER_CPCS;
    NVIC_EnableIRQ(TC0_IRQn);
}

void servo_position_stop(void)
{
    NVIC_DisableIRQ(TC0_IRQn);
    m_target_servo_position = SERVO_TARGET_POS_INVALID;
    TC0->TC_CHANNEL[0].TC_IDR = TC_IDR_CPCS;
    NVIC_EnableIRQ(TC0_IRQn);
}

void servo_position_set(uint16_t position)
{
    if (position >= SERVO_STEP_COUNT)
    {
        return;
    }

    TC0->TC_CHANNEL[0].TC_RA = TC_RA_VALUE(position + SERVO_MIN_STEPS);
}