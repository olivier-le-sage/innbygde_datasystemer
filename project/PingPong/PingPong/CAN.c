/*
 * CAN bus communication interface.
 */

#include <assert.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include "CAN.h"
#include "mcp2515.h"

// Size of the buffer for setting up a transmission.
// Equal to the TX buffer region size minus TXBnCTRL (control reg)
#define TX_BUFFER_SIZE (13)

// Size of the buffer for parsing a received message.
// Equal to the RX buffer region size minus RXBnCTRL (control) and RXBnDM (data)
#define RX_BUFFER_SIZE (5)


static can_rx_handler_t m_rx_handler;
static can_tx_handler_t m_tx_handler;

static volatile uint8_t m_tx_buf_avail;


// Allocate and take one of the available TX buffers
static int8_t m_tx_buf_alloc(void)
{
    uint8_t tx_buf_avail = m_tx_buf_avail;
    for (int8_t i = MCP_TX_BUF_COUNT - 1; i >= 0; i--)
    {
        if (tx_buf_avail >> i)
        {
            // Turn off interrupts and clear the availability bit
            cli();
            m_tx_buf_avail &= ~(_BV(i));
            sei();

            return i;
        }
    }
    return -1;
}

// Mark a TX buffer as available.
// Note: no mutual exclusion, so should be called from interrupt handler.
static inline void m_tx_buf_free(uint8_t buf)
{
    m_tx_buf_avail |= _BV(buf);
}

// Set up registers in TX buffer for transmission.
// Note: assumes that data length is in range.
static void m_tx_prepare(uint8_t buf_no, const can_id_t *id, const can_data_t *data)
{
    uint8_t buf[TX_BUFFER_SIZE];
    uint8_t buf_len;

    buf[MCP_TXBnSIDH_OFFSET] = MCP_TXBnSIDH_ENCODE(id->value);
    buf[MCP_TXBnSIDL_OFFSET] = MCP_TXBnSIDL_ENCODE(id->value, id->extended);
    buf[MCP_TXBnEID8_OFFSET] = MCP_TXBnEID8_ENCODE(id->value);
    buf[MCP_TXBnEID0_OFFSET] = MCP_TXBnEID0_ENCODE(id->value);

    if (data)
    {
        buf[MCP_TXBnDLC_OFFSET] = MCP_TXBnDLC_ENCODE(false, data->len);
        for (uint8_t i = 0; i < data->len; i++)
        {
            buf[MCP_TXBnDM_OFFSET + i] = data->data[i];
        }
        buf_len = MCP_TXBnDLC_OFFSET + 1 + data->len;
    }
    else
    {
        buf[MCP_TXBnDLC_OFFSET] = MCP_TXBnDLC_ENCODE(true, 0);
        buf_len = MCP_TXBnDLC_OFFSET + 1;
    }

    mcp_load_tx_buf_t load_buf = MCP_LOAD_TX_BUF(buf_no, false);
    mcp2515_load_tx_buffer(load_buf, buf, buf_len);
}

// Parse received data in MCP buffer into CAN message
static void m_rx_parse(uint8_t buf_no, can_msg_rx_t * msg, uint8_t * data_buf)
{
    static uint8_t buf[RX_BUFFER_SIZE];
    mcp_read_rx_buf_t read_buf = MCP_READ_RX_BUF(buf_no, false);
    bool extended;
    bool remote;

    // Read RX data up to and including DLC
    (void) mcp2515_read_rx_buffer(read_buf, buf, RX_BUFFER_SIZE);

    // Read extended bit to see if this is an extended message
    extended = (buf[MCP_RXBnSIDL_OFFSET] >> 3) & 0x01;

    // Read remote bit to see if this is a remote message
    if (!extended)
    {
        remote = (buf[MCP_RXBnSIDL_OFFSET] >> 4) & 0x01;
    }
    else
    {
        remote = (buf[MCP_RXBnDLC_OFFSET] >> 6) & 0x01;
    }

    msg->type = remote ? CAN_MSG_TYPE_REMOTE : CAN_MSG_TYPE_DATA;
    msg->id.extended = extended;
    if (!extended)
    {
        // Read standard identifier
        msg->id.value = ((buf[MCP_RXBnSIDH_OFFSET] << 3) & 0x7F8) |
                        ((buf[MCP_RXBnSIDL_OFFSET] >> 5) & 0x007);
    }
    else
    {
        // Read extended identifier
        msg->id.value = (((uint32_t) buf[MCP_RXBnSIDL_OFFSET] << 16) & 0x30000) |
                        (((uint32_t) buf[MCP_RXBnEID8_OFFSET] << 8) & 0x0FF00) |
                        (((uint32_t) buf[MCP_RXBnEID0_OFFSET]) & 0x000FF);
    }

    msg->data.len = buf[MCP_RXBnDLC_OFFSET] & 0x0F;
    // If data present, read it
    if (!remote && msg->data.len > 0)
    {
        read_buf = MCP_READ_RX_BUF(buf_no, true);
        (void) mcp2515_read_rx_buffer(read_buf, data_buf, msg->data.len);
        msg->data.data = data_buf;
    }
    else
    {
        msg->data.data = NULL;
    }
}

/* Send a message with the given ID and data
 */
static int8_t m_send(const can_id_t *id, const can_data_t *data)
{
    int8_t tx_buf = m_tx_buf_alloc();
    if (tx_buf >= 0)
    {
        // Write the message data
        m_tx_prepare(tx_buf, id, data);

        uint8_t txbctrl_addr = MCP_TXBCTRL_ADDR(tx_buf);
        // Tell the controller to send the message. Use fixed priority for now
        mcp2515_write(txbctrl_addr, MCP_TXBnCTRL_ENCODE(1, MCP_TX_PRIORITY_LOWEST));
    }

    return tx_buf;
}

// Handle completed message reception event
static void m_rx_evt_handle(uint8_t buf)
{
    static can_msg_rx_t rx_msg;
    static uint8_t rx_data_buf[MCP_DLC_MAX];

    m_rx_parse(0, &rx_msg, &rx_data_buf[0]);
    m_rx_handler(&rx_msg);
}

// Handle completed message transmission event
static void m_tx_evt_handle(uint8_t buf)
{
    m_tx_buf_free(buf);
    m_tx_handler(buf);
}

// Handle interrupts from the MCP2515
static void m_mcp2515_evt_handler(uint8_t int_flags)
{
    if (int_flags & MCP_CANINTF_RX0IF)
    {
        // RX buffer 0 was received into.
        m_rx_evt_handle(0);
    }
    if (int_flags & MCP_CANINTF_RX1IF)
    {
        // RX buffer 1 was received into.
        m_rx_evt_handle(1);
    }
    if (int_flags & MCP_CANINTF_TX0IF)
    {
        // TX buffer 0 was sent.
        m_tx_evt_handle(0);
    }
    if (int_flags & MCP_CANINTF_TX1IF)
    {
        // TX buffer 1 was sent.
        m_tx_evt_handle(1);
    }
    if (int_flags & MCP_CANINTF_TX2IF)
    {
        // TX buffer 2 was sent.
        m_tx_evt_handle(2);
    }
    if (int_flags & MCP_CANINTF_ERRIF)
    {
        // An error occurred. EFLG can be checked for further information.
        // May need some additional handling here to clear the EFLG bits.
    }
    if (int_flags & MCP_CANINTF_WAKIF)
    {
        // MCP2515 awoke from sleep due to bus activity.
    }
    if (int_flags & MCP_CANINTF_MERRF)
    {
        // A message error occurred on transmission or reception.
    }

    /* For now, clear all interrupts unconditionally */
    mcp2515_write(MCP_CANINTF, 0);
}

bool can_init(const can_init_t * init_params)
{
    assert(init_params);
    assert(init_params->rx_handler);
    assert(init_params->tx_handler);

    m_rx_handler = init_params->rx_handler;
    m_tx_handler = init_params->tx_handler;

    // Initialize TX buffer availability bitfield to all ones
    m_tx_buf_avail = (1 << MCP_TX_BUF_COUNT) - 1;

    if (!mcp2515_init(&(mcp2515_init_t){ .evt_handler = m_mcp2515_evt_handler }))
    {
        return false;
    }

    // Set mode to loop-back (change this later)
    mcp2515_bit_modify(MCP_CANCTRL, MCP_CANCTRL_MODE_MASK, MCP_CANCTRL_MODE_LOOPBACK);

	return true;
}

int8_t can_data_send(const can_id_t *id, const can_data_t *data)
{
    assert(id);
    assert(data);
    assert(data->len <= MCP_DLC_MAX);
    assert(data->len == 0 || data->data);

    return m_send(id, data);
}

int8_t can_remote_send(const can_id_t *id)
{
    assert(id);

    return m_send(id, NULL);
}

bool can_send_abort(int8_t handle)
{
    // TODO
    return false;
}
