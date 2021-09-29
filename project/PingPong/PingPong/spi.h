/*
 * Generic SPI routines
 */

#include <stdint.h>
#include <avr/io.h>

void spi_master_init(void)
{
	/* Set MOSI and SCK output, all others input */
	DDRB = (1<<DDB5)|(1<<DDB7);
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

uint8_t spi_master_read(void)
{
	return SPDR;
}