#include "mcp2515.h"
#include "spi.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include <assert.h>

ISR(INT1_vect)
{
    
}

void mcp2515_init(void)
{
    spi_master_init();

    /* Configure low level on INT1 to generate an interrupt */
    MCUCR &= ~(_BV(ISC10) | _BV(ISC11));
    /* Enable external interrupt on INT1 */
    GICR |= INT1; 
}

void mcp2515_reset(void)
{
    spi_master_enable();
    spi_master_send(MCP_RESET);
    spi_master_disable();
}

void mcp2515_read(uint8_t addr, uint8_t * data, uint8_t len)
{
    assert(data != NULL);
    assert(len > 0);

    spi_master_enable();

    spi_master_send(MCP_READ);
    spi_master_send(addr);
    data[0] = spi_master_read();

    for (uint8_t i = 1; i < len; i++)
    {
        spi_master_send(0x00);  // Send dummy data to drive SCK
        data[i] = spi_master_read();
    }

    spi_master_disable();
}

uint8_t mcp2515_read_rx_buffer(mcp_read_rx_buf_t buf, uint8_t * data, uint8_t len)
{
    assert(data != NULL);
    assert(len > 0);

    spi_master_enable();

    spi_master_send(MCP_READ_RX(buf));
    data[0] = spi_master_read();

    for (uint8_t i = 1; i < len; i++)
    {
        spi_master_send(0x00);
        data[i] = spi_master_read();
    }

    spi_master_disable();
}

void mcp2515_write(uint8_t addr, uint8_t * data, uint8_t len)
{
    assert(data != NULL);
    assert(len > 0);

    spi_master_enable();

    spi_master_send(MCP_WRITE);
    spi_master_send(addr);
    for (uint8_t i = 0; i < len; i++)
    {
        spi_master_send(data[i]);
    }

    spi_master_disable();
}

void mcp2515_load_tx_buffer(mcp_load_tx_buf_t buf, uint8_t * data, uint8_t len)
{
    assert(data != NULL);
    assert(len > 0);

    spi_master_enable();

    spi_master_send(MCP_LOAD_TX(buf));
    for (uint8_t i = 0; i < len; i++)
    {
        spi_master_send(data[i]);
    }
}

void mcp2515_request_to_send(uint8_t buf_mask)
{
    spi_master_enable();
    spi_master_send(MCP_RTS(buf_mask));
    spi_master_disable();
}

uint8_t mcp2515_read_status(void)
{
    uint8_t status;

    spi_master_enable();
    spi_master_send(MCP_READ_STATUS);
    status = spi_master_read();
    spi_master_disable();

    return status;
}

uint8_t mcp2515_rx_status(void)
{
    uint8_t status;

    spi_master_enable();
    spi_master_send(MCP_RX_STATUS);
    status = spi_master_read();
    spi_master_disable();

    return status;
}

void mcp2515_bit_modify(uint8_t addr, uint8_t mask, uint8_t data)
{
    spi_master_enable();
    spi_master_send(MCP_BITMOD);
    spi_master_send(addr);
    spi_master_send(mask);
    spi_master_send(data);
    spi_master_disable();
}
