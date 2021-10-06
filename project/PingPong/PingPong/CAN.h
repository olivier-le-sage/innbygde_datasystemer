/*
 * CAN bus communication interface.
 */

#include <stdint.h>
#include <stdbool.h>
#include "can_types.h"

typedef void (*can_rx_handler_t)(const can_msg_rx_t * msg);
typedef void (*can_tx_handler_t)(int8_t handle);

typedef struct
{
    can_rx_handler_t rx_handler;
    can_tx_handler_t tx_handler;
} can_init_t;

bool can_init(const can_init_t * init_params);
int8_t can_data_send(const can_id_t *id, const can_data_t *data);
int8_t can_remote_send(const can_id_t *id);

