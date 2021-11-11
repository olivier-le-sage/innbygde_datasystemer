#include "timer.h"

#include <sam3x8e.h>

void timer_init(void)
{
    // Assumes that servo initialized TC0
	TC0->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKDIS;
    TC0->TC_CHANNEL[1].TC_CMR = TC_CMR_TCCLKS_TIMER_CLOCK2;
}

void timer_start(void)
{
    TC0->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
}

void timer_stop(void)
{
    TC0->TC_CHANNEL[1].TC_CCR = TC_CCR_CLKDIS;
}

uint32_t timer_ms_get(void)
{
    return TC0->TC_CHANNEL[1].TC_CV / (uint32_t) MCK_8_FACTOR_MS;
}
