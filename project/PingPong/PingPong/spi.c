#include "spi.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

/*
Can we use double speed?
*/

#define PORT_SPI PORTB
#define PIN_SS PB4
#define PIN_MOSI PB5
#define PIN_MISO PB6
#define PIN_SCK PB7

static inline void m_slave_select_set(void)
{
    PORT_SPI |= _BV(PIN_SS);
}

static inline void m_slave_select_clr(void)
{
    PORT_SPI &= ~_BV(PIN_SS);
}

void spi_master_init(const spi_init_t * init_params)
{
    // Set MOSI, SCK and SS output, all others input
    DDRB = _BV(DDB4) | _BV(DDB5) | _BV(DDB7);
    // Set ~SS high initially
    m_slave_select_set();

    // Enable SPI, configure as master, set clock rate fck/16
    uint8_t spcr = _BV(SPE) | _BV(MSTR) | _BV(SPR0);
    spcr |= (init_params->data_order << DORD) & _BV(DORD);
    spcr |= (init_params->clock_polarity << CPOL) & _BV(CPOL);
    spcr |= (init_params->clock_phase << CPHA) & _BV(CPHA);
    SPCR = spcr;

    // Configure double speed
    SPSR = init_params->double_speed ? _BV(SPI2X) : 0;
}

void spi_master_enable(void)
{
    m_slave_select_clr();
}

void spi_master_disable(void)
{
    m_slave_select_set();
}

void spi_master_send(uint8_t data)
{
    // Start transmission
    SPDR = data;
    // Wait for transmission complete
    while (!(SPSR & (1 << SPIF)))
    {
    }
}

uint8_t spi_master_read(void)
{
    return SPDR;
}
