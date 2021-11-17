#ifndef MOTOR_H__
#define MOTOR_H__

#include <stdint.h>

#define MOTOR_POS_MAX_ABS (5000)

void motor_init(void);
void motor_speed_set(int32_t speed);
void motor_pos_adjust(int32_t delta);

#endif // MOTOR_H__
