#include "servo.h"
#include "pwm.h"

#include <sam3x8e.h>

#define SERVO_MIN_STEPS 90
#define SERVO_MAX_STEPS 210
#define SERVO_NEUTRAL_STEPS ((SERVO_MAX_STEPS + SERVO_MIN_STEPS) / 2)

#define SERVO_PWM_PERIOD 2000

#define SERVO_PWM_CHANNEL 0

void servo_init(void)
{
    // Signal period should be 20ms = 50Hz
    // Signal duty cycle: 0.9-2.1ms (1.5ms middle)

    // Using PREx = 8, DIVx = 105,
    // T = (8 * 105) / 84 000 000 = 0.000 01s = 0.01ms

    // With T=0.01ms we get (2.1 - 0.9) / 0.01 = 120 steps (angles)

    // Configure output pin (PC3) for PWM0HI
    PIOC->PIO_IDR = PIO_PC3B_PWMH0; // Disable interrupt on pin
    PIOC->PIO_ABSR |= PIO_PC3B_PWMH0; // Assign pin to peripheral B
    PIOC->PIO_PDR = PIO_PC3B_PWMH0;  // Let peripheral control pin

    // Configure PWM
    pwm_init_t pwm_cfg = {
        .prescaler_a = PWM_CLK_PRE_MCK_DIV_8,
        .clock_divide_factor_a = 105,
        .prescaler_b = PWM_CLK_PRE_MCK,
        .clock_divide_factor_b = PWM_CLK_CLKx_OFF, // Clock B not in use; disable
    };
    pwm_init(&pwm_cfg);

    pwm_channel_mode_t channel_mode = {
        .channel_prescaler = PWM_CLK_PRE_CLKA, // Use the PWM clock configured above
        .alignment = PWM_CH_ALIGN_LEFT,
        .polarity = PWM_CH_POLARITY_LOW,
        .counter_event_selection = PWM_CH_COUNTER_EVT_END,
    };
    pwm_channel_mode_set(SERVO_PWM_CHANNEL, &channel_mode);
    pwm_channel_period_set(SERVO_PWM_CHANNEL, SERVO_PWM_PERIOD);
    pwm_channel_duty_cycle_set(SERVO_PWM_CHANNEL, SERVO_NEUTRAL_STEPS);
    pwm_channels_enable(1 << SERVO_PWM_CHANNEL);
}
