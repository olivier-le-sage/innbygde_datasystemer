/*
 * CAN bus communication interface.
 */

#include <assert.h>
#include "CAN.h"
#include "mcp2515.h"
#include "spi.h"

void can_bus_init(void)
{
    mcp2515_init();
}

void can_bus_send(const can_bus_msg_t * message)
{
    /* In loopback configuration we can assume anything
     * sent by the master is sent back to the master.
     */
    for (uint8_t i = 0; i < message->data_len; i++)
    {
        spi_master_enable();
        spi_master_send(message->data[i]);
        uint8_t received_data = spi_master_read();
        spi_master_disable();

        // affirm loopback works correctly
        assert(received_data == message->data[i]);
    }

}


can_bus_msg_t can_bus_receive(void)
{
    // TODO
	return (can_bus_msg_t){};
}