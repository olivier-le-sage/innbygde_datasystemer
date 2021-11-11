#include "motor.h"
#include "sam.h"
#include "timer.h"

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// Parameters for the PID controller. TODO: tune
// The representation used is fixed-point with a shift of 7 (so 2^7 = 128 = 1)
#define M_FIXED_POINT_SHIFT 7
#define K_P 4  // = 0.0078125
#define K_I 1  // = 0
#define K_D 1  // = 0.0078125

#define PID_MAX_SUM_ERROR (INT32_MAX / (K_P + 1))
#define PID_MAX_ERROR     (INT32_MAX / (K_I + 1))
#define PID_MAX_I_TERM    (INT32_MAX / 2)

#define MOTOR_TIMER_INSTANCE (1)
#define PID_INTERVAL_MS (20 * MCK_8_FACTOR_MS) // TODO: tune
typedef struct
{
    int32_t  pid_controller_sum_error;
    uint32_t motor_target_pos;
    uint32_t motor_current_pos;
    uint32_t motor_last_pos;
} m_motor_pid_controller_t;

static m_motor_pid_controller_t pid_state;

int32_t m_pid_controller_next_value(void)
{
    int32_t p_term;
    int32_t i_term;
    int32_t d_term;
    int32_t error;
    int32_t temp_sum_error;
    int32_t next_adjust;

    error = (pid_state.motor_target_pos - pid_state.motor_current_pos);

    // use the error to find the next value
    if (error > PID_MAX_ERROR)
    {
        error = PID_MAX_ERROR;
    }
    else if (error < -PID_MAX_ERROR)
    {
        error = -PID_MAX_ERROR;
    }

    p_term = K_P * error;

    temp_sum_error = pid_state.pid_controller_sum_error + error;
    if (temp_sum_error > PID_MAX_SUM_ERROR)
    {
        i_term = PID_MAX_I_TERM;
        pid_state.pid_controller_sum_error = PID_MAX_SUM_ERROR;
    }
    else if (temp_sum_error < -PID_MAX_SUM_ERROR)
    {
        i_term = -PID_MAX_I_TERM;
        pid_state.pid_controller_sum_error = -PID_MAX_SUM_ERROR;
    }
    else
    {
        i_term = K_I * pid_state.pid_controller_sum_error;
        pid_state.pid_controller_sum_error = temp_sum_error;
    }

    d_term = K_D * (pid_state.motor_current_pos - pid_state.motor_last_pos);

    pid_state.motor_last_pos = pid_state.motor_current_pos;

    // Sum the P and I terms to obtain the output
    // Shift down to convert back from fixed-point numbers
    // This should work even if the terms are somehow negative,
    //   because negative numbers are represented in 2's complement on ARM
    next_adjust = (p_term + i_term + d_term) >> M_FIXED_POINT_SHIFT;

    if (pid_state.motor_current_pos + next_adjust > MOTOR_POS_MAX)
    {
        next_adjust = 0;
    }
    else if (pid_state.motor_current_pos + next_adjust < MOTOR_POS_MIN)
    {
        next_adjust = 0;
    }

    return next_adjust;
}



void m_setup_timer(void)
{
    /* Setup timer for DAC conversions
     * A new PID value is generated every PID_INTERVAL_MS ms */
    
}

void m_reset_pid_controller(void)
{
    memset(&pid_state, 0, sizeof(m_motor_pid_controller_t));
}

// DACC uses MCK/2 as clock
// Conversion -> output uses 25 clock periods
// Conversion done - interrupt
// Can utilize 4 item FIFO with word transfer mode (2x samples per write)
// Write USER_SEL in DACC_MODE to set channel
// Must configure refresh to periodically refresh output signal

static void m_dacc_value_write(uint32_t value)
{
	DACC->DACC_CDR = value;
}

void DACC_Handler(void)
{
    uint32_t status = DACC->DACC_ISR;

    if ((status & DACC_ISR_EOC) && (status & DACC_ISR_TXRDY))
    {
		int32_t pid_adjust = m_pid_controller_next_value();
		pid_state.motor_current_pos += pid_adjust;
        m_dacc_value_write(pid_state.motor_current_pos);

void TC1_Handler(void)
{
    uint32_t status = TC1->TC_CHANNEL[1].TC_SR;

    if (status & TC_SR_CPCS)
    {

        // Probably don't need to do anything here
    }
}

void motor_init(void)
{
    // Configure PMC
    // Enable clock
    PMC->PMC_PCR = PMC_PCR_PID(ID_DACC) |
                   PMC_PCR_PID(ID_TC1) |
                   PMC_PCR_CMD |
                   PMC_PCR_DIV_PERIPH_DIV_MCK |
                   PMC_PCR_EN;
    PMC->PMC_PCER1 |= (uint32_t) (1 << (ID_DACC - 32));
    PMC->PMC_PCER0 |= (uint32_t) (1 << ID_TC1);

    // I don't think we need to configure the pin in PIO since it is an "extra function"

    // TODO: configure DACC
    DACC->DACC_CR = DACC_CR_SWRST;  // Reset DACC
	  DACC->DACC_WPMR = DACC_WPMR_WPKEY(0x444143);
    DACC->DACC_MR = DACC_MR_TRGEN_EN |
                    DACC_MR_TRGSEL(0b010) | // Use TC1
                    DACC_MR_WORD_HALF |  // Use half-word mode (could maybe use word mode)
                    DACC_MR_REFRESH(1) |
                    DACC_MR_USER_SEL_CHANNEL0 |  // Use channel 0
                    DACC_MR_TAG_DIS |  // Don't use tag mode
                    DACC_MR_MAXS_NORMAL |  // Don't use max speed mode
                    DACC_MR_STARTUP_64;  // 0 clock period startup time (tune)
    // Enable channel 0
    DACC->DACC_CHER = DACC_CHER_CH0;

    // Enable interrupt on TXRDY
    DACC->DACC_IER = DACC_IER_EOC;

    NVIC_EnableIRQ(DACC_IRQn);

    m_reset_pid_controller();
    m_setup_timer();
}

void motor_pos_set(uint16_t pos)
{
    if (pos > MOTOR_POS_MAX)
    {
        return;
    }

	pid_state.motor_target_pos = pos;

    int32_t pid_adjust = m_pid_controller_next_value();
    pid_state.motor_current_pos += pid_adjust;
    m_dacc_value_write(pid_state.motor_current_pos);
}
