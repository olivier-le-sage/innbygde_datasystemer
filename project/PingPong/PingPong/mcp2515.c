#include "mcp2515_defs.h"
#include "mcp2515.h"
#include "spi.h"
#include <avr/io.h>
#include <avr/interrupt.h>

#include <assert.h>

static mcp2515_evt_handler_t m_evt_handler = NULL;

ISR(INT1_vect)
{
    // TODO: should this handler do more/less ?
    uint8_t code = mcp2515_read(MCP_CANINTF);
    m_evt_handler(code);
}

bool mcp2515_init(const mcp2515_init_t * init_params)
{
    assert(init_params);
    assert(init_params->evt_handler);

    m_evt_handler = init_params->evt_handler;

    spi_init_t spi_init = {
        .data_order = SPI_DATA_ORDER_MSB_FIRST,
        .clock_polarity = SPI_CLOCK_POLARITY_RISE_FALL,
        .clock_phase = SPI_CLOCK_PHASE_SAMPLE_SETUP,
        .double_speed = false
    };
    spi_master_init(&spi_init);

    // Configure low level on INT1 to generate an interrupt
    MCUCR &= ~(_BV(ISC10) | _BV(ISC11));
    // Enable external interrupt on INT1
    GICR |= _BV(INT1);

    // Enable interrupt generation on the MCP2515
    mcp2515_write(MCP_CANINTE,
                  MCP_CANINTE_RX0IE | MCP_CANINTE_RX1IE |
                  MCP_CANINTE_TX0IE | MCP_CANINTE_TX1IE | MCP_CANINTE_TX2IE);

    return true; 
}

void mcp2515_reset(void)
{
    spi_master_enable();

    spi_master_send(MCP_RESET);

    spi_master_disable();
}

uint8_t mcp2515_read(uint8_t addr)
{
    uint8_t data;

    spi_master_enable();

    spi_master_send(MCP_READ);
    spi_master_send(addr);

    spi_master_send(0x0);
    data = spi_master_read();

    spi_master_disable();

    return data;
}

void mcp2515_read_multiple(uint8_t addr, uint8_t * data, uint8_t len)
{
    assert(data != NULL);
    assert(len > 0);

    spi_master_enable();

    spi_master_send(MCP_READ);
    spi_master_send(addr);

    for (uint8_t i = 1; 0 < len; i++)
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

    for (uint8_t i = 0; i < len; i++)
    {
        spi_master_send(0x00);
        data[i] = spi_master_read();
    }

    spi_master_disable();

    return len;
}

void mcp2515_write(uint8_t addr, uint8_t data)
{
    spi_master_enable();

    spi_master_send(MCP_WRITE);
    spi_master_send(addr);
    spi_master_send(data),

    spi_master_disable();
}

void mcp2515_write_multiple(uint8_t addr, uint8_t * data, uint8_t len)
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

    spi_master_disable();
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

    spi_master_send(0x00);
    status = spi_master_read();

    spi_master_disable();

    return status;
}

uint8_t mcp2515_rx_status(void)
{
    uint8_t status;

    spi_master_enable();

    spi_master_send(MCP_RX_STATUS);

    spi_master_send(0x00);
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
