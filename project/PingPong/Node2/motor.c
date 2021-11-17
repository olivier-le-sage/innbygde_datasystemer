#include "motor.h"
#include "sam.h"
#include "timer.h"
#include "systick.h"

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

// SysTick default clock is MCK (?)
#define M_SYSTICK_TENMS       (0x2904 * 8)

// Parameters for the PID controller. TODO: tune
// The representation used is fixed-point with a shift of 7 (so 2^7 = 128 <=> 1)
#define M_FIXED_POINT_SHIFT 7
#define K_P 2  // = 0.6640625
#define K_I 0   // = 0.015625
#define K_D 4  // = 0.5859375

#define PID_MAX_SUM_ERROR (INT32_MAX / (K_P + 1))
#define PID_MAX_ERROR     (INT32_MAX / (K_I + 1))
#define PID_MAX_I_TERM    (INT32_MAX / 2)

#define DACC_MIN (0)
#define DACC_MAX (((1 << 12) - 1))
#define DACC_NEUT (DACC_MAX/2)

// (Arduino) pinout of the quadrature encoder control signals (MJ1)
#define M_QENC_OE_N  (1 << 0 ) // PIOD pin 0  // (arduino 32) // PIN25
#define M_QENC_RST_N (1 << 1 ) // PIOD pin 1  // (arduino 29) // PIN26
#define M_QENC_SEL   (1 << 2 ) // PIOD pin 2  // (arduino 30) // PIN27
#define M_QENC_EN    (1 << 9 ) // PIOD pin 9  // (arduino 25) // PIN30
#define M_QENC_DIR   (1 << 10) // PIOD pin 10 // (arduino 23) // PIN32

// (Arduino) pinout of the quadrature encoder data signals (MJ2)
#define M_QENC_D0 (1 << 1) // PIOC pin 1 // (arduino 24) // PIN33
#define M_QENC_D1 (1 << 2) // PIOC pin 2 // (arduino 21) // PIN34
#define M_QENC_D2 (1 << 3) // PIOC pin 3 // (arduino 22) // PIN35
#define M_QENC_D3 (1 << 4) // PIOC pin 4 // (arduino 19) // PIN36
#define M_QENC_D4 (1 << 5) // PIOC pin 5 // (arduino 20) // PIN37
#define M_QENC_D5 (1 << 6) // PIOC pin 6 // (arduino 17) // PIN38
#define M_QENC_D6 (1 << 7) // PIOC pin 7 // (arduino 18) // PIN39
#define M_QENC_D7 (1 << 8) // PIOC pin 8 // (arduino 15) // PIN40

// Maps to the state of the DIR pin. TODO: Confirm that DIR = 0 is to the right and not left
typedef enum
{
	MOTOR_DIR_RIGHT = 0,
	MOTOR_DIR_LEFT,
} m_motor_dir_t;

typedef struct
{
    int32_t pid_controller_sum_error;
    int32_t motor_target_pos;
    int32_t motor_current_pos;
    int32_t motor_last_pos;
} m_motor_pid_controller_t;

static m_motor_pid_controller_t m_pid_state;
static int16_t m_next_adjust;

static int16_t m_pid_controller_next_value(void)
{
    int32_t p_term;
    int32_t i_term;
    int32_t d_term;
    int32_t error;
    int32_t temp_sum_error;
    int16_t next_adjust;

    error = (m_pid_state.motor_target_pos - m_pid_state.motor_current_pos);

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

    temp_sum_error = m_pid_state.pid_controller_sum_error + error;
    if (temp_sum_error > PID_MAX_SUM_ERROR)
    {
        i_term = PID_MAX_I_TERM;
        m_pid_state.pid_controller_sum_error = PID_MAX_SUM_ERROR;
    }
    else if (temp_sum_error < -PID_MAX_SUM_ERROR)
    {
        i_term = -PID_MAX_I_TERM;
        m_pid_state.pid_controller_sum_error = -PID_MAX_SUM_ERROR;
    }
    else
    {
        i_term = K_I * m_pid_state.pid_controller_sum_error;
        m_pid_state.pid_controller_sum_error = temp_sum_error;
    }

    d_term = K_D * (m_pid_state.motor_last_pos - m_pid_state.motor_current_pos);

    m_pid_state.motor_last_pos = m_pid_state.motor_current_pos;

    // Sum the P and I terms to obtain the output
    // Shift down to convert back from fixed-point numbers
    next_adjust = (p_term + i_term + d_term) >> M_FIXED_POINT_SHIFT;

    return next_adjust;
}

static void m_reset_pid_controller(void)
{
    memset(&m_pid_state, 0, sizeof(m_motor_pid_controller_t));
}

static void m_dacc_write_next_value(void)
{
	DACC->DACC_CDR = m_next_adjust >> 2;
}

static void m_delay_20us(void)
{
  /* Note: Only works if not in low power/sleep mode. */
  const volatile uint32_t systick_value_at_entry = SysTick->VAL;
  volatile uint32_t systick_current_val;

  /* With a 84MHz clock, 8*210 clk cycles = 20us */
  for(;;)
  {
	  systick_current_val = SysTick->VAL;
	  
	  if (systick_current_val < systick_value_at_entry)
	  {
		  if ((systick_value_at_entry - systick_current_val) >= 210 * 8)
		  {
			  break;
		  }
	  }
	  else // Catch wraparound case (if it exists)
	  {
		 break;
	  }
  }
}

static int16_t m_read_quadrature_encoder_value(void)
{
	// Use MJ1 and MJ2 to read the current motor state.
	// Note that this requires several dozen us's delay and runs in interrupt context,
	// which is why a Systick period of less than 1ms should not be used.

	/* Routine is as follows:
	 *   - Set OE_N low to enable encoder output
	 *   - Set SEL low
	 *   - Wait ~20us
	 *   - Read MSB
	 *   - Set SEL high
	 *   - Wait ~20us
	 *   - Read LSB
	 *   - Toggle RST_N (this will set the encoder's counter back to 0)
	 *   - Set OE_N high
	 */

  PIOD->PIO_CODR |= M_QENC_OE_N | M_QENC_SEL;
  m_delay_20us();
  const volatile uint8_t qenc_msb = (PIOC->PIO_PDSR >> 1) & 0xFF;

  PIOD->PIO_SODR |= M_QENC_SEL;
  m_delay_20us();
  const volatile uint8_t qenc_lsb = (PIOC->PIO_PDSR >> 1) & 0xFF;

  PIOD->PIO_CODR |= M_QENC_RST_N; // Need to reset encoder value
  m_delay_20us();
  PIOD->PIO_SODR |= M_QENC_RST_N;

  PIOD->PIO_SODR |= M_QENC_OE_N;

  // Knowing that the ARM architecture uses 2's complement to represent signed
  // integers, we can safely cast the return value to int16_t 
  return (int16_t)((qenc_msb << 8) | qenc_lsb);
}

static void m_systick_handle(void)
{
	const int16_t quadrature_encoder_value = m_read_quadrature_encoder_value();
	
	/* The value of the quadrature encoder counter indicates how much the motor has moved since
	 * the last time it was polled.
	 * Motor goes to the right -> negative encoder value */
    m_pid_state.motor_current_pos += quadrature_encoder_value;

	const volatile int16_t pid_adjust = m_pid_controller_next_value();

	if (pid_adjust < 0)
	{
		PIOD->PIO_SODR |= M_QENC_DIR;
		m_next_adjust = pid_adjust;
	}
	else
	{
		PIOD->PIO_CODR |= M_QENC_DIR;
		m_next_adjust = -1 * pid_adjust;
	}

	m_dacc_write_next_value();
}

void DACC_Handler(void)
{
    uint32_t status = DACC->DACC_ISR;

    if ((status & DACC_ISR_EOC) && (status & DACC_ISR_TXRDY))
    {
		m_dacc_write_next_value();
	}
}

static void m_setup_pios(void)
{
  PMC->PMC_PCR |= PMC_PCR_PID(ID_PIOC) |
				  PMC_PCR_CMD |
				  PMC_PCR_DIV_PERIPH_DIV_MCK |
				  PMC_PCR_EN;
  PMC->PMC_PCER0 |= ID_PIOC;
  
  PMC->PMC_PCR |= PMC_PCR_PID(ID_PIOD) |
                  PMC_PCR_CMD |
                  PMC_PCR_DIV_PERIPH_DIV_MCK |
                  PMC_PCR_EN;
  PMC->PMC_PCER0 |= ID_PIOD;

  /* Configure i/o settings for the motor and encoder. */
  const uint32_t piod_outputs = M_QENC_OE_N  |
                                M_QENC_RST_N |
                                M_QENC_SEL   |
                                M_QENC_EN    |
                                M_QENC_DIR;
  PIOD->PIO_PER |= piod_outputs;
  PIOD->PIO_PDR &= ~piod_outputs;
  PIOD->PIO_OER = piod_outputs;
  PIOD->PIO_ODR = ~piod_outputs;
  PIOD->PIO_PUDR = (uint32_t)-1;
  PIOD->PIO_SODR |= M_QENC_EN; // Enable the motor
  PIOD->PIO_SODR |= M_QENC_OE_N | M_QENC_RST_N; // Drive !OE and !RST high -- they are active-low

  const uint32_t pioc_inputs = M_QENC_D0
                             | M_QENC_D1
                             | M_QENC_D2
                             | M_QENC_D3
                             | M_QENC_D4
                             | M_QENC_D5
                             | M_QENC_D6
                             | M_QENC_D7;
  PIOC->PIO_PER = (uint32_t)-1;
  PIOC->PIO_PDR = 0;
  PIOC->PIO_OER = ~pioc_inputs; // configure unused pins as output so that they stay 0
  PIOC->PIO_IFER = pioc_inputs;
  PIOC->PIO_IFDR = ~pioc_inputs;
  PIOC->PIO_PUER = 0;
  PIOC->PIO_PUDR = (uint32_t)-1;

  // Toggle reset once to ensure the counter is 0
  PIOD->PIO_CODR |= M_QENC_RST_N;
  m_delay_20us();
  PIOD->PIO_SODR |= M_QENC_RST_N;
}

void motor_init(void)
{
	m_next_adjust = 0;

    // Configure PMC
    // Enable clock - use programmable clock so that the DACC output doesn't change too fast
    PMC->PMC_PCR |= PMC_PCR_PID(ID_DACC) |
                    PMC_PCR_CMD |
                    PMC_PCR_DIV_PERIPH_DIV_MCK |
                    PMC_PCR_EN;
    PMC->PMC_PCER1 |= (uint32_t) (1 << (ID_DACC - 32));

    // I don't think we need to configure the pin in PIO since it is an "extra function"

    // TODO: configure DACC
    DACC->DACC_CR = DACC_CR_SWRST;  // Reset DACC
	DACC->DACC_WPMR = DACC_WPMR_WPKEY(0x444143);
    DACC->DACC_MR = DACC_MR_TRGEN_DIS |
                    DACC_MR_WORD_HALF |  // Use half-word mode (could maybe use word mode)
                    DACC_MR_REFRESH(32) |
                    DACC_MR_USER_SEL_CHANNEL0 |  // Use channel 0
                    DACC_MR_TAG_DIS |  // Don't use tag mode
                    DACC_MR_MAXS_NORMAL |  // Don't use max speed mode
                    DACC_MR_STARTUP_0;  // 0 clock period startup time (tune)

    // Enable channel 0
    DACC->DACC_CHER = DACC_CHER_CH0;

    // Enable interrupt on EOC
    DACC->DACC_IER = DACC_IER_EOC;

	NVIC_EnableIRQ(DACC_IRQn);

	m_setup_pios();
	m_reset_pid_controller();
	
    (void) systick_cb_register(m_systick_handle);

	/*
	for (uint16_t i = 0; i < 10000; i++)
	{
		m_delay_20us();
	}
	*/
	
	// DACC module will send the motor flying to the right at the beginning.
	// We will use the right side as a reference by resetting the counter after some delay.
	PIOD->PIO_CODR |= M_QENC_RST_N; // Need to reset encoder value
	m_delay_20us();
	PIOD->PIO_SODR |= M_QENC_RST_N;
}


/* Implements speed-based control (open-loop control without positioning) */
void motor_speed_set(int32_t speed)
{
	if (speed < 0)
	{
		m_next_adjust = (uint32_t) (-1 * speed);
		PIOD->PIO_CODR |= M_QENC_DIR;
	}
	else
	{
		m_next_adjust = (uint32_t)speed;
		PIOD->PIO_SODR |= M_QENC_DIR;
	}

	m_pid_state.pid_controller_sum_error = 0; // reset the integrator
}

void motor_pos_adjust(int32_t delta)
{
	m_pid_state.motor_target_pos += delta;
	m_pid_state.pid_controller_sum_error = 0; // reset the integrator
}
