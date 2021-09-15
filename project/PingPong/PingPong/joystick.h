/*
 * joystick.h - Joystick driver
 *
 * Created: 15.09.2021 11:52:41
 *  Author: oliviel
 */ 


#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <stdint.h>

typedef enum
{
	NEUTRAL = 0,
	LEFT,
	RIGHT,
	UP,
	DOWN,
} joystick_direction_t;

typedef struct
{
	joystick_direction_t first_dir;
	joystick_direction_t second_dir;
	uint16_t             x;
	uint16_t             y;
} joystick_position_t;

void joystick_init(void);
void get_joystick_pos(joystick_position_t *p_joystick_direction_out);
void get_joystick_dir(joystick_direction_t *p_first_dir_out, joystick_direction_t *p_second_dir_out);


#endif /* JOYSTICK_H_ */