/*
 * can_controller.c
 *
 * Author: Gustav O. Often and Eivind H. J�lsgard
 *
 * For use in TTK4155 Embedded and Industrial Computer Systems Design
 * NTNU - Norwegian University of Science and Technology
 *
 */

#include "CAN.h"

#include "sam.h"

#include "printf_stdarg.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define DEBUG_INTERRUPT 0

#define F_MCK (84000000)  // 84MHz

#define BRP_CALCULATE(baudrate) ((uint32_t) (F_MCK) / ((uint32_t) baudrate) - 1)

static can_rx_handler_t m_rx_handler;
static can_tx_handler_t m_tx_handler;

static uint8_t m_rx_buf_count;
static uint8_t m_tx_buf_count;

static void m_rx_parse(uint8_t buf_no, can_msg_rx_t *msg, uint8_t *data)
{
    //Get data from CAN mailbox
    uint32_t data_low = CAN0->CAN_MB[buf_no].CAN_MDL;
    uint32_t data_high = CAN0->CAN_MB[buf_no].CAN_MDH;

    //Get message ID
    msg->id.value = (uint16_t)((CAN0->CAN_MB[buf_no].CAN_MID & CAN_MID_MIDvA_Msk) >> CAN_MID_MIDvA_Pos);

    // TODO: support remote
    msg->type = CAN_MSG_TYPE_DATA;

    //Get data length
    msg->data.len = (uint8_t)((CAN0->CAN_MB[buf_no].CAN_MSR & CAN_MSR_MDLC_Msk) >> CAN_MSR_MDLC_Pos);

    //Put data in CAN_MESSAGE object
    for (int i = 0; i < MIN(msg->data.len, 4); i++)
    {
        data[i] = (char)(data_low & 0xff);
        data_low = data_low >> 8;
    }
    for (int i = 4; i < MIN(msg->data.len, 8); i++)
    {
        data[i] = (uint8_t)(data_high & 0xff);
        data_high = data_high >> 8;
    }

    msg->data.data = data;

    //Reset for new receive
    CAN0->CAN_MB[buf_no].CAN_MMR = CAN_MMR_MOT_MB_RX;
    CAN0->CAN_MB[buf_no].CAN_MCR |= CAN_MCR_MTCR;
}

static void m_rx_evt_handle(uint8_t buf_no)
{
    static can_msg_rx_t rx_msg;
    static uint8_t rx_data_buf[8];

    // Double check that mailbox is ready
    if (CAN0->CAN_MB[buf_no].CAN_MSR & CAN_MSR_MRDY)
    {
        m_rx_parse(buf_no, &rx_msg, &rx_data_buf[0]);
        m_rx_handler(buf_no, &rx_msg);
    }
}

void CAN0_Handler(void)
{
    if (DEBUG_INTERRUPT)
    {
        uart_printf("CAN0 interrupt\n\r");
    }

    char can_sr = CAN0->CAN_SR;

	for (uint8_t tx_buf = 0; tx_buf < m_tx_buf_count; tx_buf++)
	{
		if (can_sr & (1 << tx_buf))
		{
			//Disable interrupt
			CAN0->CAN_IDR = (1 << tx_buf);
		}
	}

	for (uint8_t rx_buf = m_tx_buf_count; rx_buf < m_rx_buf_count + m_tx_buf_count; rx_buf++)
	{
		if (can_sr & (1 << rx_buf))
		{
			m_rx_evt_handle(rx_buf);
		}
	}
    if (can_sr & CAN_SR_ERRP)
    {
        if (DEBUG_INTERRUPT) {
			uart_printf("CAN0 ERRP error\n\r");
		}
    }
    if (can_sr & CAN_SR_TOVF)
    {
    	if (DEBUG_INTERRUPT) {
			uart_printf("CAN0 timer overflow\n\r");
		}
    }

    NVIC_ClearPendingIRQ(ID_CAN0);
    //sei();*/
}

/**
 * \brief Initialize can bus
 *
 * \param can_br Value to be set in CAN0->CAN_BR register to match can bus bit timing
 *
 * \param num_tx_mb Number of transmit mailboxes,     tx mb indexes: [0        , num_tx_mb-1]
 *
 * \param num_rx_mb Number of receive mailboxes,     rx mb indexes: [num_tx_mb, num_rx_mb-1]
 *
 * \retval Success(0) or failure(1)
 */
uint8_t can_init(const can_init_t * init_params)
{
	if (!init_params ||
		!init_params->rx_handler ||
		!init_params->tx_handler ||
		init_params->buf.rx_buf_count > 8 ||
		init_params->buf.tx_buf_count > 8 ||
		init_params->buf.tx_buf_count + init_params->buf.tx_buf_count > 8)
	{
		return CAN_ERROR_INVALID;
	}

    uint32_t brp = BRP_CALCULATE(init_params->bit.baudrate);
	if (brp < 1 || 0x7F < brp)
	{
		return CAN_ERROR_INVALID;
	}

    m_rx_handler = init_params->rx_handler;
    m_tx_handler = init_params->tx_handler;

    uint32_t ul_status;
	(void)ul_status;

    //Disable can
    CAN0->CAN_MR &= ~CAN_MR_CANEN;
    //Clear status register on read
    ul_status = CAN0->CAN_SR;

    // Disable interrupts on CANH and CANL pins
    PIOA->PIO_IDR = PIO_PA8A_URXD | PIO_PA9A_UTXD;

    //Select CAN0 RX and TX in PIOA
    uint32_t ul_sr = PIOA->PIO_ABSR;
    PIOA->PIO_ABSR = ~(PIO_PA1A_CANRX0 | PIO_PA0A_CANTX0) & ul_sr;

    // Disable the Parallel IO (PIO) of the Rx and Tx pins so that the peripheral controller can use them
    PIOA->PIO_PDR = PIO_PA1A_CANRX0 | PIO_PA0A_CANTX0;

    // Enable pull up on CANH and CANL pin
    PIOA->PIO_PUER = (PIO_PA1A_CANRX0 | PIO_PA0A_CANTX0);

    //Enable Clock for CAN0 in PMC
    PMC->PMC_PCR = PMC_PCR_EN | (0 << PMC_PCR_DIV_Pos) | PMC_PCR_CMD | (ID_CAN0 << PMC_PCR_PID_Pos); // DIV = 1(can clk = MCK/2), CMD = 1 (write), PID = 2B (CAN0)
    PMC->PMC_PCER1 |= 1 << (ID_CAN0 - 32);

    //Set baudrate, Phase1, phase2 and propagation delay for can bus. Must match on all nodes!
    CAN0->CAN_BR = CAN_BR_PHASE2(init_params->bit.phase_2_len - 1) |
                   CAN_BR_PHASE1(init_params->bit.phase_1_len - 1) |
                   CAN_BR_PROPAG(init_params->bit.prop_seg_len - 1) |
                   CAN_BR_SJW(init_params->bit.sync_jump_len - 1) |
                   CAN_BR_BRP(brp) |
                   CAN_BR_SMP_ONCE;

    /****** Start of mailbox configuration ******/
    m_rx_buf_count = init_params->buf.rx_buf_count;
    m_tx_buf_count = init_params->buf.tx_buf_count;

    uint32_t can_ier = 0;

    /*Configure transmit mailboxes */
    for (int n = 0; n < m_tx_buf_count; n++)
    {
        CAN0->CAN_MB[n].CAN_MID = CAN_MID_MIDE;
        CAN0->CAN_MB[n].CAN_MMR = (CAN_MMR_MOT_MB_TX);

		can_ier |= 1 << n; //Enable interrupt on tx mailboxes
    }

    /* Configure receive mailboxes */
    for (int n = m_tx_buf_count;
         n < m_rx_buf_count + m_tx_buf_count;
         n++)  //Simply one mailbox setup for all messages. You might want to apply filter for them.
    {
        CAN0->CAN_MB[n].CAN_MAM = 0; //Accept all messages
        CAN0->CAN_MB[n].CAN_MID = CAN_MID_MIDE;
        CAN0->CAN_MB[n].CAN_MMR = (CAN_MMR_MOT_MB_RX);
        CAN0->CAN_MB[n].CAN_MCR |= CAN_MCR_MTCR;

		can_ier |= 1 << n; //Enable interrupt on rx mailboxes
    }

    /****** End of mailbox configuraion ******/

    //Enable interrupt on receive mailboxes
    CAN0->CAN_IER = can_ier;

    //Enable interrupt in NVIC
    NVIC_EnableIRQ(ID_CAN0);

    //enable CAN
    CAN0->CAN_MR |= CAN_MR_CANEN;

    return 0;
}

/**
 * \brief Send can message from mailbox
 *
 * \param can_msg message to be sent
 *
 * \param tx_mb_id ID of transmit mailbox to be used
 *
 * \retval Success(0) or failure(1)
 */
uint8_t can_data_send(uint8_t tx_buf_no, const can_id_t *id, const can_data_t *data)
{

	if (tx_buf_no > m_tx_buf_count ||
		!id ||
		!data ||
		(data->len > 0 && !data->data) ||
		data->len > 8)
	{
		return CAN_ERROR_INVALID;
	}

    if (id->extended)
    {
        return CAN_ERROR_NOT_SUPPORTED;
    }

    //Check that mailbox is ready
    if (!(CAN0->CAN_MB[tx_buf_no].CAN_MSR & CAN_MSR_MRDY))
    {
        return CAN_ERROR_BUSY;
    }

    //Set message ID and use CAN 2.0B protocol
    CAN0->CAN_MB[tx_buf_no].CAN_MID = CAN_MID_MIDvA(id->value) | CAN_MID_MIDE ;

    //Put message in can data registers
    uint32_t can_mdl = 0;
    uint32_t can_mdh = 0;

    for (int i = 0; i < MIN(4, data->len); i++)
    {
        can_mdl |= (data->data[i] << (8 * i));
    }
    for (int i = 4; i < MIN(8, data->len); i++)
    {
        can_mdh |= (data->data[i] << (8 * (i - 4)));
    }

    CAN0->CAN_MB[tx_buf_no].CAN_MDL = can_mdl;
    CAN0->CAN_MB[tx_buf_no].CAN_MDH = can_mdh;

    //Set message length and mailbox ready to send
    CAN0->CAN_MB[tx_buf_no].CAN_MCR = CAN_MCR_MDLC(data->len) | CAN_MCR_MTCR;

    return CAN_SUCCESS;
}

uint8_t can_get_error_counters(can_error_counter_t * counts)
{
    uint32_t ecr = CAN0->CAN_ECR;
    counts->tec = (ecr & CAN_ECR_TEC_Msk) >> CAN_ECR_TEC_Pos;
    counts->rec = (ecr & CAN_ECR_REC_Msk) >> CAN_ECR_REC_Pos;

    return CAN_SUCCESS;
}
