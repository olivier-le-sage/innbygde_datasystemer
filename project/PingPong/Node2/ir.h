/*
 * ir.h
 *
 * Created: 27.10.2021 17:53:38
 *  Author: oliviel
 */

#ifndef IR_H_
#define IR_H_

#include <stdint.h>

typedef enum
{
	IR_NOT_BLOCKED = 0,
	IR_BLOCKED,
} ir_state_t;

void ir_adc_init(void);
uint32_t ir_blocked_count_get(void);
ir_state_t ir_state_get(void);
void ir_blocked_count_reset(void);

#endif /* IR_H_ */
