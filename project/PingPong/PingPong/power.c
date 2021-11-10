#include "ping_pong.h"
#include "power.h"

void power_sleep(void)
{
    // Configure sleep mode
    EMCUCR &= ~(1 << SM0);
    MCUCSR &= ~(1 << SM2);
    MCUCR &= ~(1 << SM1);

    // TODO: should ACSR be made to disable analog comparator?

    // Enable sleep
    MCUCR |= (1 << SE);
    __asm__("sleep");
    MCUCR &= ~(1 << SE);
}