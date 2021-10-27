#ifndef SERVO_H__
#define SERVO_H__

#include <stdint.h>

// Initialize the servo
void servo_init(void);
// Adjust the servo by a certain delta
void servo_position_adjust(int16_t delta);
// Set the servo position in the allowed range [0, 120)
void servo_position_set(uint16_t position);

#endif // SERVO_H__
