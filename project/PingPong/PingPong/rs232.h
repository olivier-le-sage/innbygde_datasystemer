/* Routines for printing to the RS-232 output.
 * Use UART.
 */

#include "ping_pong.h"

bool uart_init(void);
char uart_fetch_by_force(void);
void uart_config_streams(void);
void uart_print(char *string);