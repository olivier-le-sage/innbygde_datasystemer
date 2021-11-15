#include "systick.h"
#include "sam.h"

// SysTick default clock is MCK (?)
#define M_SYSTICK_TENMS       (0x2904 * 8)
#define M_SYSTICK_EVERY_100MS ((M_SYSTICK_TENMS - 1) * 100)

static systick_cb_t m_cb;

void SysTick_Handler(void)
{
	// Vi faar ikke systick IRQ mens CPU sover.
	// Bruker vi det klokt, er det fremdeles miljoevennlig :)
    if (m_cb)
    {
        m_cb();
    }
}

void systick_init(const systick_init_t * params)
{
    uint32_t load_value;

    m_cb = params->cb;
    load_value = params->period_10ms * (M_SYSTICK_TENMS - 1);
    // assert(load_value > 0 && load_value <= SysTick_LOAD_RELOAD_Msk);

    // Configure Systick settings
	SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk
				   | SysTick_CTRL_ENABLE_Msk
				   | SysTick_CTRL_TICKINT_Msk;
	SysTick->LOAD = load_value & SysTick_LOAD_RELOAD_Msk;
}