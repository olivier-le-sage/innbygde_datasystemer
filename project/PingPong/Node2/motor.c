#include "motor.h"
#include "sam.h"

#include <stdbool.h>
#include <string.h>

// Parameters for the PI controller. TODO: tune
// The representation used is fixed-point with a shift of 5
#define K_P 32 // = 1
#define K_I 16 // = 0.5

#define PI_MAX_SUM_ERROR (INT32_MAX / (K_P + 1))
#define PI_MAX_ERROR     (INT32_MAX / (K_I + 1))
#define PI_MAX_I_TERM    (0)
static int32_t m_pi_controller_sum_error;
static int32_t m_pi_controller_error;
static uint32_t m_motor_target_pos;
static uint32_t m_motor_current_pos;

typedef struct
{
    int32_t  pi_controller_sum_error;
    int32_t  pi_controller_error;
    uint32_t motor_target_pos;
    uint32_t motor_current_pos;
} m_motor_pi_controller_t;

static m_motor_pi_controller_t pi_state;

uint16_t m_pi_controller_next_value(void)
{
    uint32_t p_term;
    uint32_t i_term;
    int32_t error;
    int32_t temp_sum_error;
    int32_t next_value;

    error = pi_state.motor_target_pos - pi_state.motor_current_pos;

    // use the error to find the next value    
    if (error > PI_MAX_ERROR)
    {
        error = PI_MAX_ERROR;
    }
    else if (error < -PI_MAX_ERROR)
    {
        error = -PI_MAX_ERROR;
    }

    p_term = K_P * error;

    temp_sum_error = pi_state.pi_controller_sum_error + error;
    if (temp_sum_error > PI_MAX_SUM_ERROR)
    {
        i_term = PI_MAX_I_TERM;
        pi_state.pi_controller_sum_error = PI_MAX_SUM_ERROR;
    }
    else if (temp_sum_error < -PI_MAX_SUM_ERROR)
    {
        i_term = -MAX_I_TERM;
        pi_state.pi_controller_sum_error = -PI_MAX_SUM_ERROR;
    }
    else
    {
        i_term = K_I * m_pi_controller_sum_error;
        pi_state.pi_controller_sum_error = temp;
    }

    // Sum the P and I terms to obtain the output
    // Shift down to convert back from fixed-point numbers
    // This should work even if the terms are somehow negative, 
    //   because negative numbers are represented in 2's complement on ARM
    next_value = (p_term + i_term) >> 5;

    if (next_value > MOTOR_POS_MAX)
    {
        next_value = MOTOR_POS_MAX;
    }
    else if (next_value < MOTOR_POS_MIN)
    {
        next_value = MOTOR_POS_MIN;
    }

    return (uint16_t)next_value;
}

void m_reset_pi_controller(void)
{
    memset(&pi_state, 0, sizeof(m_motor_pi_controller_t));
}

// DACC uses MCK/2 as clock
// Conversion -> output uses 25 clock periods
// Conversion done - interrupt
// Can utilize 4 item FIFO with word transfer mode (2x samples per write)
// Write USER_SEL in DACC_MODE to set channel
// Must configure refresh to periodically refresh output signal

static void m_dacc_value_write(uint16_t value)
{
	DACC->DACC_CDR = (uint32_t) value;
}

void DACC_Handler(void)
{
    uint32_t status = DACC->DACC_ISR;

    if ((status & DACC_ISR_EOC) && (status & DACC_ISR_TXRDY))
    {
        m_dacc_value_write(m_pi_controller_next_value());
    }
}

void motor_init(void)
{
    // Configure PMC
    // Enable clock
    PMC->PMC_PCR = PMC_PCR_PID(ID_DACC) |
                   PMC_PCR_CMD |
                   PMC_PCR_DIV_PERIPH_DIV_MCK |
                   PMC_PCR_EN;
    PMC->PMC_PCER1 |= (uint32_t) (1 << (ID_DACC - 32));

    // I don't think we need to configure the pin in PIO since it is an "extra function"

    // TODO: configure DACC
    DACC->DACC_CR = DACC_CR_SWRST;  // Reset DACC
	DACC->DACC_WPMR = DACC_WPMR_WPKEY(0x444143);
    DACC->DACC_MR = DACC_MR_TRGEN_DIS |  // Use free-running mode
                    // DAC_MR_TRGSEL(<val>) |
                    DACC_MR_WORD_HALF |  // Use half-word mode (could maybe use word mode)
                    DACC_MR_REFRESH(128) |  // Refresh every 1024 * x / (MCK / 2) cycles (tune)
                    DACC_MR_USER_SEL_CHANNEL0 |  // Use channel 0
                    DACC_MR_TAG_DIS |  // Don't use tag mode
                    DACC_MR_MAXS_NORMAL |  // Don't use max speed mode
                    DACC_MR_STARTUP_64;  // 0 clock period startup time (tune)
    // Enable channel 0
    DACC->DACC_CHER = DACC_CHER_CH0;

    // Enable interrupt on TXRDY
    DACC->DACC_IER = DACC_IER_EOC;

    NVIC_EnableIRQ(DACC_IRQn);

    m_reset_pi_controller();
}

void motor_pos_set(uint16_t pos)
{
    if (pos > MOTOR_POS_MAX)
    {
        return;
    }

    m_motor_target_pos = pos;
}
