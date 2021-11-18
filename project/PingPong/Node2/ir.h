/*
 * ir.h
 *
 * Created: 27.10.2021 17:53:38
 *  Author: oliviel
 */

#ifndef IR_H_
#define IR_H_

#include <stdint.h>
#include <stdbool.h>

void ir_adc_init(void);
bool ir_triggered_get_reset(void);

#endif /* IR_H_ */
