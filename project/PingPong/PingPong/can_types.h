#ifndef CAN_TYPES_H__
#define CAN_TYPES_H__

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    uint32_t value;
    bool extended;
} can_id_t;

typedef struct
{
    uint8_t len;
    const uint8_t *data;
} can_data_t;

typedef enum
{
    CAN_MSG_TYPE_DATA,
    CAN_MSG_TYPE_REMOTE,
} can_msg_type_t;

typedef struct
{
    can_msg_type_t type;
    can_id_t id;
    can_data_t data;
} can_msg_rx_t;

#endif /* CAN_TYPES_H__*/