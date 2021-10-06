/*
 * Generic SPI routines
 */
#ifndef SPI_H__
#define SPI_H__

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    SPI_DATA_ORDER_MSB_FIRST = 0b0,
    SPI_DATA_ORDER_LSB_FIRST = 0b1
} spi_data_order_t;

typedef enum
{
    SPI_CLOCK_POLARITY_RISE_FALL = 0,
    SPI_CLOCK_POLARITY_FALL_RISE = 1
} spi_clock_polarity_t;

typedef enum
{
    SPI_CLOCK_PHASE_SAMPLE_SETUP = 0,
    SPI_CLOCK_PHASE_SETUP_SAMPLE = 1
} spi_clock_phase_t;

typedef struct
{
    spi_data_order_t data_order;
    spi_clock_polarity_t clock_polarity;
    spi_clock_phase_t clock_phase;
    bool double_speed;
} spi_init_t;

void spi_master_init(const spi_init_t * init_params);

void spi_master_enable(void);
void spi_master_disable(void);

void spi_master_send(uint8_t data);
uint8_t spi_master_read(void);

#endif /* SPI_H__ */
