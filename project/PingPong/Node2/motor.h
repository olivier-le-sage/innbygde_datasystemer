#ifndef MOTOR_H__
#define MOTOR_H__

#include <stdint.h>

void motor_init(void);
void motor_pos_set(int32_t pos);
void motor_pos_adjust(int32_t delta);
void motor_systick_handle(void);

#endif // MOTOR_H__
