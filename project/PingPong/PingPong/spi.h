/*
 * Generic SPI routines
 */
#ifndef SPI_H__
#define SPI_H__

#include <stdint.h>
#include <stdbool.h>

void spi_master_init(void);

void spi_master_enable(void);
void spi_master_disable(void);

void spi_master_send(uint8_t data);
uint8_t spi_master_read(void);

#endif /* SPI_H__ */
