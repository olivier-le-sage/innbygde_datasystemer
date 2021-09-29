#include "spi.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

/*
Can we use double speed?
MCP2515 clocks data in on rising SCK, out on falling SCK
So we need CPOL=0, CPHA=1
*/

#define PORT_SPI PORTB
#define PIN_SS PB4
#define PIN_MOSI PB5
#define PIN_MISO PB6
#define PIN_SCK PB7

static inline void m_slave_select_set(void)
{
    PORT_SPI |= PIN_SS;
}

static inline void m_slave_select_clr(void)
{
    PORT_SPI &= ~PIN_SS;
}

void spi_master_init(void)
{
    /* Set MOSI, SCK and SS output, all others input */
    DDRB = _BV(DDB4) | _BV(DDB5) | _BV(DDB7);
    /* Enable SPI, configure as master, set clock rate fck/16 */
    SPCR = _BV(SPE) | _BV(MSTR) | _BV(SPR0);
    /* Set ~SS high initially */
    m_slave_select_set();
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
    /* Start transmission */
    SPDR = data;
    /* Wait for transmission complete */
    while (!(SPSR & (1 << SPIF)))
    {
    }
}

uint8_t spi_master_read(void)
{
    return SPDR;
}
