/*
 * CAN bus communication interface.
 */

#include <stdint.h>
#include <stdbool.h>
#include "can_types.h"

#define CAN_BUF_INVALID 0xFF

#define CAN_SUCCESS 0
#define CAN_ERROR_INVALID 1
#define CAN_ERROR_BUSY 2
#define CAN_ERROR_NOT_SUPPORTED 10
#define CAN_ERROR_GENERIC 50

typedef void (*can_rx_handler_t)(uint8_t rx_buf_no, const can_msg_rx_t * msg);
typedef void (*can_tx_handler_t)(uint8_t tx_buf_no);

typedef struct
{
    uint32_t baudrate;
    uint8_t sync_jump_len;
    uint8_t prop_seg_len;
    uint8_t phase_1_len;
    uint8_t phase_2_len;
} can_bit_timing_t;

typedef struct
{
    uint8_t rx_buf_count;
    uint8_t tx_buf_count;
} can_buf_cfg_t;

typedef struct
{
    // Handler function for message reception
    can_rx_handler_t rx_handler;
    // Handler function for transmission complete events
    can_tx_handler_t tx_handler;
    can_buf_cfg_t buf;
    can_bit_timing_t bit;
} can_init_t;

// Initialize CAN driver module
uint8_t can_init(const can_init_t * init_params);
// Send a CAN data message
uint8_t can_data_send(uint8_t tx_buf_no, const can_id_t *id, const can_data_t *data);
// Send a CAN remote message (data request)
uint8_t can_remote_send(uint8_t tx_buf_no, const can_id_t *id);

uint8_t can_get_error_counters(can_error_counter_t * counts);