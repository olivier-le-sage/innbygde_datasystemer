#include "power.h"
#include "sam.h"

void power_sleep(void)
{
	// Configure sleep mode (sleep)
	SCB->SCR &= ~(SCB_SCR_SLEEPDEEP_Msk); // Set SLEEPDEEP = 0
	PMC->PMC_FSMR &= ~(PMC_FSMR_LPM); // Set LPM = 0

	// Make sure pipeline is empty (unsure if necessary)
    __ISB();
    __DSB();

	// Go to sleep
	__WFE();
}