/*
 * Generic SPI routines
 */

#include <stdint.h>
#include <avr/io.h>

void spi_master_init(void)
{
	/* Set MOSI and SCK output, all others input */
	DDR_SPI = (1<<DD_MOSI)|(1<<DD_SCK);
	/* Enable SPI, configure as master, set clock rate fck/16 */
	SPCR = (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void spi_master_send(uint8_t byte)
{
	/* Start transmission */
	SPDR = byte;
	/* Wait for transmission complete */
	while(!(SPSR & (1<<SPIF)));
}