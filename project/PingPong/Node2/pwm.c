
#include "pwm.h"

#include <sam3x8e.h>


// Handle PWM interrupt
// Handle FAULT? maybe not necessary
// Configure correct clock (i.e. allow user to set 20ms period)
// Configure channel (clock, etc.)

// Channel period: (PRESC * DIVA_B * CPRD) / MCK
// (use CPRD to configure)

// Channel duty cycle: (period - CDTY / f_channel_x_clock) / period
// (use CDTY to configure)

// Channel polarity: L/H
// Channel waveform alignment: left/center

void PWM_Handler(void)
{

}

void pwm_init(const pwm_init_t * init) 
{
        
    // Unlock User Interface by writing the WPCMD field in the PWM_WPCR Register.
    if (!init ||
        init->prescaler_a > PWM_CLK_PRE_MCK_DIV_1024 ||
        init->prescaler_b > PWM_CLK_PRE_MCK_DIV_1024)
    {
        return;
    }

    // Enable PWM clock in PMC
    PMC->PMC_PCR = PMC_PCR_EN | (0 << PMC_PCR_DIV_Pos) | PMC_PCR_CMD | (ID_PWM << PMC_PCR_PID_Pos);
    PMC->PMC_PCER1 |= 1 << (ID_PWM - 32);

    // Configuration of the clock generator (DIVA, PREA, DIVB, PREB in the PWM_CLK register if required).
    PWM->PWM_CLK = PWM_CLK_DIVA(init->clock_divide_factor_a) |
                   PWM_CLK_PREA((uint32_t) init->prescaler_a) |
                   PWM_CLK_DIVB(init->clock_divide_factor_b) |
                   PWM_CLK_PREB((uint32_t) init->prescaler_b);


    // Configuration of the period for each channel (CPRD in the PWM_CPRDx register). Writing in PWM_CPRDx
    //   register is possible while the channel is disabled. After validation of the channel, the user must use

    //   PWM_CPRDUPDx register to update PWM_CPRDx as explained below.

    // Configuration of the duty-cycle for each channel (CDTY in the PWM_CDTYx register). Writing in
    //   PWM_CDTYx register is possible while the channel is disabled. After validation of the channel, the user
    //   must use PWM_CDTYUPDx register to update PWM_CDTYx as explained below.

    // Configuration of the dead-time generator for each channel (DTH and DTL in PWM_DTx) if enabled (DTE bit
    //   in the PWM_CMRx register). Writing in the PWM_DTx register is possible while the channel is disabled.

    // After validation of the channel, the user must use PWM_DTUPDx register to update PWM_DTx

    // Selection of the synchronous channels (SYNCx in the PWM_SCM register)

    // Selection of the moment when the WRDY flag and the corresponding PDC transfer request are set (PTRM
    //   and PTRCS in the PWM_SCM register)

    // Configuration of the update mode (UPDM in the PWM_SCM register)

    // Configuration of the update period (UPR in the PWM_SCUP register) if needed.

    // Configuration of the comparisons (PWM_CMPVx and PWM_CMPMx).

    // Configuration of the event lines (PWM_ELMRx).

    // Configuration of the fault inputs polarity (FPOL in PWM_FMR)

    // Configuration of the fault protection (FMOD and FFIL in PWM_FMR, PWM_FPV and PWM_FPE/2)

    // Enable of the Interrupts (writing CHIDx and FCHIDx in PWM_IER1 register, and writing WRDYE, ENDTXE,
    //   TXBUFE, UNRE, CMPMx and CMPUx in PWM_IER2 register)

    // Enable of the PWM channels (writing CHIDx in the PWM_ENA register)


}


void pwm_channels_enable(uint8_t channels)
{
    PWM->PWM_ENA = (uint32_t) channels;
}

void pwm_channels_disable(uint8_t channels)
{
    PWM->PWM_DIS = (uint32_t) channels;
}

void pwm_channel_mode_set(uint8_t channel, const pwm_channel_mode_t * cfg)
{
    // Selection of the clock for each channel (CPRE field in the PWM_CMRx register)
    // Configuration of the waveform alignment for each channel (CALG field in the PWM_CMRx register)
    // Selection of the counter event selection (if CALG = 1) for each channel (CES field in the PWM_CMRx
    //   register)
    // Configuration of the output waveform polarity for each channel (CPOL in the PWM_CMRx register)
    if (channel > PWMCH_NUM_NUMBER ||
        !cfg ||
        cfg->channel_prescaler > PWM_CLK_PRE_CLKB ||
        cfg->alignment > PWM_CH_ALIGN_CENTER ||
        cfg->polarity > PWM_CH_POLARITY_HIGH ||
        cfg->counter_event_selection > PWM_CH_COUNTER_EVT_END_HALF)
    {
        return;
    }

    // Ignore dead times, etc.
    PWM->PWM_CH_NUM[channel].PWM_CMR = (((uint32_t) cfg->channel_prescaler << PWM_CMR_CPRE_Pos) & PWM_CMR_CPRE_Msk) |
                                       (((uint32_t) cfg->alignment << 8) & PWM_CMR_CALG) |
                                       (((uint32_t) cfg->polarity << 9) & PWM_CMR_CPOL) |
                                       (((uint32_t) cfg->counter_event_selection) & PWM_CMR_DTE); 
}

void pwm_channel_duty_cycle_set(uint8_t channel, uint16_t duty_cycle)
{
    PWM->PWM_CH_NUM[channel].PWM_CDTY = PWM_CDTY_CDTY(channel);
}

void pwm_channel_duty_cycle_update(uint8_t channel, uint16_t duty_cycle)
{
    PWM->PWM_CH_NUM[channel].PWM_CDTYUPD = PWM_CDTYUPD_CDTYUPD(channel);
}

void pwm_channel_period_set(uint8_t channel, uint16_t period)
{
    PWM->PWM_CH_NUM[channel].PWM_CPRD = PWM_CPRD_CPRD(period);
}

void pwm_channel_period_update(uint8_t channel, uint16_t period)
{
    PWM->PWM_CH_NUM[channel].PWM_CPRDUPD = PWM_CPRDUPD_CPRDUPD(period);
}
