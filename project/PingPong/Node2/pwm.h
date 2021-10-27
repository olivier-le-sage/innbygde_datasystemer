#ifndef PWM_H__
#define PWM_H__

#include <stdint.h>

#define PWM_CLK_CLKx_OFF 0

typedef enum
{
    PWM_CLK_PRE_MCK = 0,
    PWM_CLK_PRE_MCK_DIV_2,
    PWM_CLK_PRE_MCK_DIV_4,
    PWM_CLK_PRE_MCK_DIV_8,
    PWM_CLK_PRE_MCK_DIV_16,
    PWM_CLK_PRE_MCK_DIV_32,
    PWM_CLK_PRE_MCK_DIV_64,
    PWM_CLK_PRE_MCK_DIV_128,
    PWM_CLK_PRE_MCK_DIV_256,
    PWM_CLK_PRE_MCK_DIV_512,
    PWM_CLK_PRE_MCK_DIV_1024,
    PWM_CLK_PRE_CLKA,
    PWM_CLK_PRE_CLKB,
} pwm_prescaler_t;

typedef struct
{
    pwm_prescaler_t prescaler_a;
    uint8_t clock_divide_factor_a;
    pwm_prescaler_t prescaler_b;
    uint8_t clock_divide_factor_b;
} pwm_init_t;

typedef enum
{
    PWM_CH_ALIGN_LEFT = 0,
    PWM_CH_ALIGN_CENTER
} pwm_ch_align_t;

typedef enum
{
    PWM_CH_POLARITY_LOW = 0,
    PWM_CH_POLARITY_HIGH
} pwm_ch_polarity_t;

typedef enum
{
    PWM_CH_COUNTER_EVT_END = 0,
    PWM_CH_COUNTER_EVT_END_HALF
} pwm_ch_counter_evt_t;

typedef struct
{
    pwm_prescaler_t channel_prescaler;
    pwm_ch_align_t alignment;
    pwm_ch_polarity_t polarity;
    pwm_ch_counter_evt_t counter_event_selection;
} pwm_channel_mode_t;


void pwm_init(const pwm_init_t * init);

void pwm_channels_enable(uint8_t channels);
void pwm_channels_disable(uint8_t channels);

void pwm_channel_mode_set(uint8_t channel, const pwm_channel_mode_t * cfg);


void pwm_channel_duty_cycle_set(uint8_t channel, uint16_t duty_cycle);
void pwm_channel_duty_cycle_update(uint8_t channel, uint16_t duty_cycle);

void pwm_channel_period_set(uint8_t channel, uint16_t period);
void pwm_channel_period_update(uint8_t channel, uint16_t period);

#endif // PWM_H__
