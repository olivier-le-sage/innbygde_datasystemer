/* Routines for printing to the RS-232 output.
 * Use UART.
 */

#include "ping_pong.h"

// After calling this function it becomes possible to make printf() calls
// Those printf() calls will output to UART
bool uart_init(void);

char uart_fetch_by_force(void);