/* Routines for printing to the RS-232 output.
 * Use UART.
 */

/*
UMSEL to select sync/async
UBBR for baudrate (UBBR = fOsc/(16 BAUD) - 1)
*/

#include <stdio.h>
#include <avr/sfr_defs.h>
#include <avr/io.h>
#include "rs232.h"

#define FOSC 1843200  /* Clock speed */
#define BAUDRATE 9600

static const unsigned int m_ubbr_value = ((FOSC / 16) / BAUDRATE - 1);

/* Function prototypes */
static int m_uart_printchar(char char_to_print, FILE *stream);
static int m_uart_getchar(FILE *stream);

// allocate output stream statically to avoid malloc()
static FILE uart_stream = FDEV_SETUP_STREAM(m_uart_printchar, m_uart_getchar, _FDEV_SETUP_WRITE);

static int m_uart_printchar(char char_to_print, FILE *stream)
{
    if (char_to_print == '\n')
    {
        return m_uart_printchar('\r', stream);
    }

    // busy-wait until bit UDRE is set in UCSRA
    while ( bit_is_clear(UCSR0A, UDRE0) );

    UDR0 = char_to_print; // write out character

    return 0;
}

static int m_uart_getchar(FILE *stream)
{
    // there should be no uart stream errors.
    assert(ferror(&uart_stream));

    if (feof(&uart_stream)) // test for end-of-file
    {
        // not sure how to handle; assert for now
        assert(false);
        return 0;
    }
    else
    {
        return fgetc(&uart_stream);
    }
}

bool uart_init(void)
{
	/* Set baud rate */
	UBRR0H = (uint8_t) (m_ubbr_value >> 8);
	UBRR0L = (uint8_t) m_ubbr_value;

    stdout = &uart_stream;
    return true;
}
