#ifndef MCP2515_H__
#define MCP2515_H__

#include "mcp2515_defs.h"
#include <stdbool.h>

typedef void (*mcp2515_evt_handler_t)(uint8_t int_flags);
typedef struct
{
    mcp2515_evt_handler_t evt_handler;
} mcp2515_init_t;

bool mcp2515_init(const mcp2515_init_t * init_params);
void mcp2515_reset(void);
uint8_t mcp2515_read(uint8_t addr);
void mcp2515_read_multiple(uint8_t addr, uint8_t * data, uint8_t len);
uint8_t mcp2515_read_rx_buffer(mcp_read_rx_buf_t buf, uint8_t * data, uint8_t len);
void mcp2515_write(uint8_t addr, uint8_t data);
void mcp2515_write_multiple(uint8_t addr, uint8_t * data, uint8_t len);
void mcp2515_load_tx_buffer(mcp_load_tx_buf_t buf, uint8_t * data, uint8_t len);
void mcp2515_request_to_send(uint8_t buf_mask);
uint8_t mcp2515_read_status(void);
uint8_t mcp2515_rx_status(void);
void mcp2515_bit_modify(uint8_t addr, uint8_t mask, uint8_t data);

#endif /* MCP2515_H__ */