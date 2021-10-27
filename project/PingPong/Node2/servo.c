#include "servo.h"
#include "pwm.h"

// Signal period should be 20ms = 50Hz
// Signal duty cycle: 0.9-2.1ms (1.5ms middle)

// 84 Mhz / 256 = 328 125 Hz
// 

void servo_init(void)
{
    const uint8_t servo_pwm_channel = 0;

    /*
    pwm_init_t pwm_params = {
        .prescaler_a = PWM_CLK_PRE_MCK_DIV_256,
        .clock_divide_factor_a = 



    }
    */

}
