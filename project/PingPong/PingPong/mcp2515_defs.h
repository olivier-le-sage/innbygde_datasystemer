/*
mcp2515.h

This file contains constants that are specific to the MCP2515.

Version     Date        Description
----------------------------------------------------------------------
v1.00       2003/12/11  Initial release

Copyright 2003 Kimberly Otten Software Consulting
*/

#ifndef MCP2515_DEFS_H__
#define MCP2515_DEFS_H__

#include <stdint.h>
#include "utils.h"

// Define MCP2515 register addresses

#define MCP_RXF0SIDH	0x00
#define MCP_RXF0SIDL	0x01
#define MCP_RXF0EID8	0x02
#define MCP_RXF0EID0	0x03
#define MCP_RXF1SIDH	0x04
#define MCP_RXF1SIDL	0x05
#define MCP_RXF1EID8	0x06
#define MCP_RXF1EID0	0x07
#define MCP_RXF2SIDH	0x08
#define MCP_RXF2SIDL	0x09
#define MCP_RXF2EID8	0x0A
#define MCP_RXF2EID0	0x0B
#define MCP_CANSTAT		0x0E
#define MCP_CANCTRL		0x0F
#define MCP_RXF3SIDH	0x10
#define MCP_RXF3SIDL	0x11
#define MCP_RXF3EID8	0x12
#define MCP_RXF3EID0	0x13
#define MCP_RXF4SIDH	0x14
#define MCP_RXF4SIDL	0x15
#define MCP_RXF4EID8	0x16
#define MCP_RXF4EID0	0x17
#define MCP_RXF5SIDH	0x18
#define MCP_RXF5SIDL	0x19
#define MCP_RXF5EID8	0x1A
#define MCP_RXF5EID0	0x1B
#define MCP_TEC			0x1C
#define MCP_REC			0x1D
#define MCP_RXM0SIDH	0x20
#define MCP_RXM0SIDL	0x21
#define MCP_RXM0EID8	0x22
#define MCP_RXM0EID0	0x23
#define MCP_RXM1SIDH	0x24
#define MCP_RXM1SIDL	0x25
#define MCP_RXM1EID8	0x26
#define MCP_RXM1EID0	0x27
#define MCP_CNF3		0x28
#define MCP_CNF2		0x29
#define MCP_CNF1		0x2A
#define MCP_CANINTE		0x2B
#define MCP_CANINTF		0x2C
#define MCP_EFLG		0x2D
#define MCP_TXB0CTRL	0x30
#define MCP_TXB1CTRL	0x40
#define MCP_TXB2CTRL	0x50
#define MCP_RXB0CTRL	0x60
#define MCP_RXB0SIDH	0x61
#define MCP_RXB1CTRL	0x70
#define MCP_RXB1SIDH	0x71

#define MCP_RXBnSIDH_OFFSET 0
#define MCP_RXBnSIDL_OFFSET 1
#define MCP_RXBnEID8_OFFSET 2
#define MCP_RXBnEID0_OFFSET 3
#define MCP_RXBnDLC_OFFSET 4
#define MCP_RXBnDM_OFFSET 5

#define MCP_TXBnSIDH_OFFSET 0
#define MCP_TXBnSIDL_OFFSET 1
#define MCP_TXBnEID8_OFFSET 2
#define MCP_TXBnEID0_OFFSET 3
#define MCP_TXBnDLC_OFFSET 4
#define MCP_TXBnDM_OFFSET 5

#define MCP_TX_INT		0x1C		// Enable all transmit interrupts
#define MCP_TX01_INT	0x0C		// Enable TXB0 and TXB1 interrupts
#define MCP_RX_INT		0x03		// Enable receive interrupts
#define MCP_NO_INT		0x00		// Disable all interrupts

#define MCP_TX01_MASK	0x14
#define MCP_TX_MASK		0x54

// Define SPI Instruction Set
#define MCP_WRITE		0x02
#define MCP_READ		0x03
#define MCP_BITMOD		0x05
#define MCP_LOAD_TX0	0x40
#define MCP_LOAD_TX1	0x42
#define MCP_LOAD_TX2	0x44

#define MCP_LOAD_TX(buf) \
    _FORCE_UINT8(0x40 | ((buf) & 0x07))

#define MCP_RTS(bufs) \
    _FORCE_UINT8(0x80 | ((bufs) & 0x07))

#define MCP_READ_RX(buf) \
    _FORCE_UINT8(0x90 | (((buf) << 1) & 0x06))

#define MCP_READ_STATUS	0xA0
#define MCP_RX_STATUS	0xB0
#define MCP_RESET		0xC0

typedef enum
{
    MCP_TX_BUF_0_TXB0SIDH = 0b000,
    MCP_TX_BUF_0_TXB0D0 = 0b001,
    MCP_TX_BUF_1_TXB1SIDH = 0b010,
    MCP_TX_BUF_1_TXB1D0 = 0b011,
    MCP_TX_BUF_2_TXB2SIDH = 0b100,
    MCP_TX_BUF_2_TXB2D0 = 0b101
} mcp_load_tx_buf_t;

typedef enum
{
    MCP_RX_BUF_0_RXB0SIDH = 0b00,
    MCP_RX_BUF_0_RXB0D0 = 0b01,
    MCP_RX_BUF_1_RXB1SIDH = 0b10,
    MCP_RX_BUF_1_RXB1D0 = 0b11
} mcp_read_rx_buf_t;

#define MCP_TX_BUF_COUNT 3
#define MCP_RX_BUF_COUNT 2

/*
11 = Highest Message Priority
10 = High Intermediate Message Priority
01 = Low Intermediate Message Priority
00 = Lowest Message Priority
*/

typedef enum
{
    MCP_TX_PRIORITY_LOWEST = 0b00,
    MCP_TX_PRIORITY_LOW_INTERMEDIATE = 0b01,
    MCP_TX_PRIORITY_HIGH_INTERMEDIATE = 0b10,
    MCP_TX_PRIORITY_HIGHEST = 0b11
} mcp_tx_priority_t;

#define MCP_DLC_MAX 8

#define MCP_TXBCTRL_ADDR(buf_no) (MCP_TXB0CTRL + ((buf_no) << 4))
#define MCP_LOAD_TX_BUF(buf_no, d0) (((buf_no) << 1) | ((d0) & 0x01))
#define MCP_READ_RX_BUF(buf_no, d0) (((buf_no) << 1) | ((d0) & 0x01))

// CANSTAT Register values
#define MCP_CANSTAT_MODE_NORMAL     0x00
#define MCP_CANSTAT_MODE_SLEEP      0x20
#define MCP_CANSTAT_MODE_LOOPBACK   0x40
#define MCP_CANSTAT_MODE_LISTENONLY 0x60
#define MCP_CANSTAT_MODE_CONFIG     0x80
#define MCP_CANSTAT_MODE_MASK		0xE0
#define MCP_CANSTAT_INT_NONE        0x00
#define MCP_CANSTAT_INT_ERR         0x02
#define MCP_CANSTAT_INT_WAK         0x04
#define MCP_CANSTAT_INT_TXB0        0x06
#define MCP_CANSTAT_INT_TXB1        0x08
#define MCP_CANSTAT_INT_TXB2        0x0A
#define MCP_CANSTAT_INT_RXB0        0x0C
#define MCP_CANSTAT_INT_RXB1        0x0E
#define MCP_CANSTAT_INT_MASK        0x0E

// CANCTRL Register Values
#define MCP_CANCTRL_MODE_NORMAL     0x00
#define MCP_CANCTRL_MODE_SLEEP      0x20
#define MCP_CANCTRL_MODE_LOOPBACK   0x40
#define MCP_CANCTRL_MODE_LISTENONLY 0x60
#define MCP_CANCTRL_MODE_CONFIG     0x80
#define MCP_CANCTRL_MODE_POWERUP	0xE0
#define MCP_CANCTRL_MODE_MASK		0xE0
#define MCP_CANCTRL_ABORT_TX        0x10
#define MCP_CANCTRL_MODE_ONESHOT	0x08
#define MCP_CANCTRL_CLKOUT_ENABLE	0x04
#define MCP_CANCTRL_CLKOUT_DISABLE	0x00
#define MCP_CANCTRL_CLKOUT_PS1		0x00
#define MCP_CANCTRL_CLKOUT_PS2		0x01
#define MCP_CANCTRL_CLKOUT_PS4		0x02
#define MCP_CANCTRL_CLKOUT_PS8		0x03

// CNF1 Register Values
#define MCP_CNF1_SJW1            0x00
#define MCP_CNF1_SJW2            0x40
#define MCP_CNF1_SJW3            0x80
#define MCP_CNF1_SJW4            0xC0

// CNF2 Register Values
#define MCP_CNF2_BTLMODE		 0x80
#define MCP_CNF2_SAMPLE_1X       0x00
#define MCP_CNF2_SAMPLE_3X       0x40

// CNF3 Register Values
#define MCP_CNF3_SOF_ENABLE		0x80
#define MCP_CNF3_SOF_DISABLE	0x00
#define MCP_CNF3_WAKFIL_ENABLE	0x40
#define MCP_CNF3_WAKFIL_DISABLE	0x00

// CANINTE register bits
#define MCP_CANINTE_RX0IE		0x01
#define MCP_CANINTE_RX1IE		0x02
#define MCP_CANINTE_TX0IE		0x04
#define MCP_CANINTE_TX1IE   	0x08
#define MCP_CANINTE_TX2IE		0x10
#define MCP_CANINTE_ERRIE		0x20
#define MCP_CANINTE_WAKIE		0x40
#define MCP_CANINTE_MERRE		0x80

// CANINTF Register Bits
#define MCP_CANINTF_RX0IF		0x01
#define MCP_CANINTF_RX1IF		0x02
#define MCP_CANINTF_TX0IF		0x04
#define MCP_CANINTF_TX1IF   	0x08
#define MCP_CANINTF_TX2IF		0x10
#define MCP_CANINTF_ERRIF		0x20
#define MCP_CANINTF_WAKIF		0x40
#define MCP_CANINTF_MERRF		0x80

/* Encode register value for TXBnCTRL */
#define MCP_TXBnCTRL_ENCODE(txreq, priority) \
    _FORCE_UINT8((((txreq) << 3) & 0x08) | ((priority)&0x03))

/* Encode register value for TXBnSIDH */
#define MCP_TXBnSIDH_ENCODE(id) \
    _FORCE_UINT8((id) >> 3)

/* Encode register value for TXBnSIDL */
#define MCP_TXBnSIDL_ENCODE(id, extended) \
    _FORCE_UINT8((((id) << 5) & 0xE0) | (((extended) << 3) & 0x08) | (((id) >> 16) & 0x03))

/* Encode register value for TXBnEID8 */
#define MCP_TXBnEID8_ENCODE(id) \
    _FORCE_UINT8((id) >> 8)

/* Encode register value for TXBnEID0 */
#define MCP_TXBnEID0_ENCODE(id) \
    _FORCE_UINT8(id)

/* Encode register value for TXBnDLC */
#define MCP_TXBnDLC_ENCODE(remote, len) \
    _FORCE_UINT8((((remote) << 6) & 0x40) | ((len)&0x0F))



#endif /* MCP2515_DEFS_H__ */