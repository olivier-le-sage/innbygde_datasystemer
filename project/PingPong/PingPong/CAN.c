/*
 * CAN bus communication interface.
 */

#include <assert.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <avr/sfr_defs.h>
#include "CAN.h"
#include "mcp2515.h"
#include "ping_pong.h"

// TODO: enable sleep mode?

// Size of the buffer for setting up a transmission.
// Equal to the TX buffer region size minus TXBnCTRL (control reg)
#define TX_BUFFER_SIZE (13)

// Size of the buffer for parsing a received message.
// Equal to the RX buffer region size minus RXBnCTRL (control) and RXBnDM (data)
#define RX_BUFFER_SIZE (5)

#define F_MCP_CPU (16000000) // 16MHz

// Information processing time in TQ, starting at ps2
#define IPT_TQ (2)

// Calculate BRP based on baudrate and CPU frequency
#define BRP_CALCULATE(baudrate) ((uint8_t) ((((uint32_t)(F_MCP_CPU / 2)  / (baudrate)) - 1) & 0x3F))

// Output pin connected to the MCP2551 Rs pin
#define TRANSCEIVER_RS_PIN (PB3)

static can_rx_handler_t m_rx_handler;
static can_tx_handler_t m_tx_handler;

static volatile uint8_t m_tx_buf_avail;


// Allocate and take the buffer with number buf_no
static bool m_tx_buf_take(uint8_t buf_no)
{
    if (m_tx_buf_avail & _BV(buf_no))
    {
        cli();
        m_tx_buf_avail &= ~(_BV(buf_no));
        sei();
        return true;
    }

    return false;
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
static uint8_t m_send(uint8_t tx_buf_no, const can_id_t *id, const can_data_t *data)
{
    if (m_tx_buf_take(tx_buf_no))
    {
        // Write the message data
        m_tx_prepare(tx_buf_no, id, data);

        uint8_t txbctrl_addr = MCP_TXBCTRL_ADDR(tx_buf_no);
        // Tell the controller to send the message. Use fixed priority for now
        mcp2515_write(txbctrl_addr, MCP_TXBnCTRL_ENCODE(1, MCP_TX_PRIORITY_LOWEST));

        return CAN_SUCCESS;
    }

    return CAN_ERROR_BUSY;
}

// Handle completed message reception event
static void m_rx_evt_handle(uint8_t buf)
{
    static can_msg_rx_t rx_msg;
    static uint8_t rx_data_buf[MCP_DLC_MAX];

    m_rx_parse(buf, &rx_msg, &rx_data_buf[0]);
    m_rx_handler(buf, &rx_msg);
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

    // For now, clear all interrupts unconditionally
    mcp2515_write(MCP_CANINTF, 0);
}

uint8_t can_init(const can_init_t * init_params)
{
    assert(init_params);
    assert(init_params->rx_handler);
    assert(init_params->tx_handler);

    const can_bit_timing_t * bit_cfg = &init_params->bit;

    /// Check bit timing allowed values
    assert(1 <= bit_cfg->sync_jump_len && bit_cfg->sync_jump_len <= 4);
    assert(1 <= bit_cfg->prop_seg_len && bit_cfg->prop_seg_len <= 8);
    assert(1 <= bit_cfg->phase_1_len && bit_cfg->phase_1_len <= 8);
    assert(IPT_TQ <= bit_cfg->phase_2_len && bit_cfg->phase_2_len <= 8);

    // Check bit timing requirements; see mcp2515 sec. 5.3
    assert(bit_cfg->prop_seg_len + bit_cfg->phase_1_len >= bit_cfg->phase_2_len);
    assert(bit_cfg->phase_2_len > bit_cfg->sync_jump_len);

    m_rx_handler = init_params->rx_handler;
    m_tx_handler = init_params->tx_handler;

    // Initialize TX buffer availability bitfield to all ones
    m_tx_buf_avail = (1 << MCP_TX_BUF_COUNT) - 1;

    // Set up MCP2551 Rs pin and set it low to enable the transceiver
    DDRB |= (1 << TRANSCEIVER_RS_PIN);
    PORTB &= ~(1 << TRANSCEIVER_RS_PIN);

    if (!mcp2515_init(&(mcp2515_init_t){ .evt_handler = m_mcp2515_evt_handler }))
    {
        return CAN_ERROR_GENERIC;
    }

    // Ensure configuration mode
    mcp2515_bit_modify(MCP_CANCTRL, MCP_CANCTRL_MODE_MASK, MCP_CANCTRL_MODE_CONFIG);

	// Configure bit timing configuration
    uint8_t brp = BRP_CALCULATE(init_params->bit.baudrate);
    uint8_t cnf1 = MCP_CNF1_ENCODE(init_params->bit.sync_jump_len - 1, brp);
    uint8_t cnf2 = MCP_CNF2_ENCODE(1, 0, init_params->bit.phase_1_len - 1,
                                   init_params->bit.prop_seg_len - 1);
    uint8_t cnf3 = MCP_CNF3_ENCODE(1, 0, init_params->bit.phase_2_len - 1);

	mcp2515_write(MCP_CNF1, cnf1); // BRP = 2*TQ
	mcp2515_write(MCP_CNF2, cnf2);
	mcp2515_write(MCP_CNF3, cnf3);

	// Verify configuration (validate write order)
	assert(cnf1 == mcp2515_read(MCP_CNF1));
	assert(cnf2 == mcp2515_read(MCP_CNF2));
	assert(cnf3 == mcp2515_read(MCP_CNF3));

    // Set normal mode
    uint8_t canctrl = MCP_CANCTRL_MODE_NORMAL |
                      MCP_CANCTRL_CLKOUT_DISABLE |
                      MCP_CANCTRL_CLKOUT_PS1;
    mcp2515_write(MCP_CANCTRL, canctrl);

	return CAN_SUCCESS;
}

uint8_t can_data_send(uint8_t tx_buf_no, const can_id_t *id, const can_data_t *data)
{
    assert(tx_buf_no < MCP_TX_BUF_COUNT);
    assert(id);
    assert(data);
    assert(data->len <= MCP_DLC_MAX);
    assert(data->len == 0 || data->data);

    return m_send(tx_buf_no, id, data);
}

uint8_t can_remote_send(uint8_t tx_buf_no, const can_id_t *id)
{
    assert(tx_buf_no < MCP_TX_BUF_COUNT);
    assert(id);

    return m_send(tx_buf_no, id, NULL);
}

bool can_send_abort(int8_t handle)
{
    // TODO
    return false;
}

uint8_t can_get_error_counters(can_error_counter_t * counts)
{
    uint8_t buf[2];

    assert(counts);

    mcp2515_read_multiple(MCP_TEC, &buf[0], 2);
    counts->tec = buf[0];
    counts->rec = buf[1];

    return CAN_SUCCESS;
}

static uint8_t m_can_mode_set(uint8_t mode)
{
    mcp2515_bit_modify(MCP_CANCTRL, MCP_CANCTRL_MODE_MASK, mode);
    uint8_t read_mode = mcp2515_read(MCP_CANCTRL) & MCP_CANCTRL_MODE_MASK;
    if (read_mode != mode)
    {
        return CAN_ERROR_WRITE;
    }
    return CAN_SUCCESS;
}

uint8_t can_sleep(void)
{
    // TODO: check if we are doing something important
    // Set mode to SLEEP
    uint8_t rc = m_can_mode_set(MCP_CANCTRL_MODE_SLEEP);
    if (rc == CAN_SUCCESS)
    {
        // Disable the transceiver by setting Rs pin high
        PORTB |= (1 << TRANSCEIVER_RS_PIN);
    }
    return rc;
}

uint8_t can_wake(void)
{
    // Set mode to NORMAL (mode is LISTENONLY after waking up)
    uint8_t rc = m_can_mode_set(MCP_CANCTRL_MODE_NORMAL);
    if (rc == CAN_SUCCESS)
    {
        // Enable the transceiver by setting Rs pin low
        PORTB &= ~(1 << TRANSCEIVER_RS_PIN);
    }
    return rc;
}

/*
The OST maintains Reset
for the first 128 OSC1 clock cycles after power-up or a
wake-up from Sleep mode occurs. It should be noted
that no SPI protocol operations should be attempted
until after the OST has expired 
*/