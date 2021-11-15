#ifndef MOTOR_H__
#define MOTOR_H__

#include <stdint.h>

#define MOTOR_POS_MIN (0)
#define MOTOR_POS_MAX (((1 << 12) - 1))

void motor_init(void);
void motor_pos_set(uint16_t pos);
void motor_pos_adjust(int16_t delta);
void motor_systick_handle(void);

#endif // MOTOR_H__