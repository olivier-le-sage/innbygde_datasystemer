#ifndef SERVO_H__
#define SERVO_H__

// Initialize the servo
void servo_init(void);
// Set the servo position in the allowed range [0, 120)
void servo_position_set(uint16_t position);

#endif // SERVO_H__
