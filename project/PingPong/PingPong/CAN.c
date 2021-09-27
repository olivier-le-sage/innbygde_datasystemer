/*
 * CAN bus communication interface.
 */

#include "CAN.h"
#include "mcp2515.h"
#include "spi.h"

void can_bus_init(void)
{
    spi_master_init();
}


void can_bus_send(can_bus_msg_t message)
{
    /* In loopback configuration we can assume anything
     * sent by the master is sent back to the master.
     */

    for (uint8_t i = 0; i < message.data_len; i++)
    {
        spi_master_send(message.data[i]);
        uint8_t received_data = spi_master_read();

        // affirm loopback works correctly
        assert(received_data == message.data[i]);
    }

}


can_bus_msg_t can_bus_receive(void)
{
    // TODO
}