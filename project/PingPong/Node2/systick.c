#include "systick.h"
#include "sam.h"

// SysTick default clock is MCK (?)
#define M_SYSTICK_TENMS       (0x2904)
#define M_SYSTICK_EVERY_100MS ((M_SYSTICK_TENMS - 1) * 100)

#define M_MAX_CALLBACKS 5

static systick_cb_t m_callbacks[M_MAX_CALLBACKS];
static uint32_t m_callback_count;
static uint32_t m_period_10ms;

void SysTick_Handler(void)
{
	// Vi faar ikke systick IRQ mens CPU sover.
	// Bruker vi det klokt, er det fremdeles miljoevennlig :)
    for (uint32_t i = 0; i < m_callback_count; ++i)
    {
        m_callbacks[i]();
    }
}

void systick_init(const systick_init_t * params)
{
    m_period_10ms = params->period_10ms;

    // Clear and turn off SysTick
    SysTick->CTRL = 0;
    SysTick->VAL = 0;

    NVIC_EnableIRQ(SysTick_IRQn);
}

void systick_enable(void)
{
    uint32_t load_value = m_period_10ms * (M_SYSTICK_TENMS - 1);

    // Configure Systick settings
	SysTick->LOAD = load_value & SysTick_LOAD_RELOAD_Msk;
    // Start SysTick with MCK/8 clock
	SysTick->CTRL = SysTick_CTRL_TICKINT_Msk |
                    SysTick_CTRL_ENABLE_Msk;
}

uint32_t systick_period_10ms_get(void)
{
    return m_period_10ms;
}

bool systick_cb_register(systick_cb_t cb)
{
    if (cb && m_callback_count < M_MAX_CALLBACKS)
    {
        m_callbacks[m_callback_count] = cb;
        m_callback_count++;
    }
    else
    {
        return false;
    }
}