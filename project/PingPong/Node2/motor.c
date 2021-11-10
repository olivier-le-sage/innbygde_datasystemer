#include "motor.h"
#include "sam.h"


// DACC uses MCK/2 as clock
// Conversion -> output uses 25 clock periods
// Conversion done - interrupt
// Can utilize 4 item FIFO with word transfer mode (2x samples per write)
// Write USER_SEL in DACC_MODE to set channel
// Must configure refresh to periodically refresh output signal

static void 


void DACC_IRQHandler(void)
{
    uint32_t status = DACC->ISR;

    if (status & DACC_ISR_TXRDY)
    {
        // TODO: handle TXRDY (load more data)
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
    PMC->PMC_PCER1 |= (uint32_t) (ID_DACC - 32);

    // I don't think we need to configure the pin in PIO since it is an "extra function"

    // TODO: configure DACC
    DACC->DACC_CR = DACC_CR_SWRST;  // Reset DACC
    DACC->DACC_MR = DACC_MR_TRGEN_DIS |  // Use free-running mode (?)
                    // DAC_MR_TRGSEL(<val>) | 
                    DACC_MR_WORD_WORD |  // Use word mode
                    DACC_MR_REFRESH(128) |  // Refresh every 1024 * x / (MCK / 2) cycles (tune)
                    DACC_MR_USER_SEL_CHANNEL0 |  // Use channel 0
                    DACC_MR_TAG_DIS |  // Don't use tag mode
                    DACC_MR_MAXS_NORMAL |  // Don't use max speed mode
                    DACC_MR_STARTUP_0;  // 0 clock period startup time (tune)
    // Enable channel 0
    DACC->DACC_CHER = DACC_CHER_CH0;

    // Enable interrupt on TXRDY
    DACC->DACC_IER = DACC_IER_TXRDY;

    NVIC_EnableIRQ(DACC_IRQn);
}
