/*
 * CAN bus communication interface.
 */

#include <stdint.h>

typedef struct
{
	uint8_t id;
	uint8_t data_len;
	uint8_t *data;
} can_bus_msg_t;

void can_bus_init(void);
void can_bus_send(can_bus_msg_t message);
can_bus_msg_t can_bus_receive(void);