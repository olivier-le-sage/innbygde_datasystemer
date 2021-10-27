/*
 * joystick.h - Joystick driver
 *
 * Created: 15.09.2021 11:52:41
 *  Author: oliviel
 */


#ifndef JOYSTICK_H_
#define JOYSTICK_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
	NEUTRAL = 0,
	LEFT    = 1,
	RIGHT   = 2,
	UP      = 3,
	DOWN    = 4,
} joystick_direction_t;

typedef struct
{
	uint8_t x;
	uint8_t y;
} joystick_position_t;

typedef struct
{
	uint8_t right_slider_pos;
	uint8_t left_slider_pos;
} sliders_position_t;

static const char * joystick_dir_to_str(joystick_direction_t joystick_dir)
{
	switch(joystick_dir)
	{
		case NEUTRAL:
			return "NEUTRAL";
			break;
		case LEFT:
			return "LEFT";
			break;
		case RIGHT:
			return "RIGHT";
			break;
		case UP:
			return "UP";
			break;
		case DOWN:
			return "DOWN";
			break;
		default:
			return "";
			break;
	}
}

bool joystick_init(void);
void get_joystick_pos(joystick_position_t *p_joystick_position_out);
void get_joystick_dir(joystick_direction_t *p_first_dir_out, joystick_direction_t *p_second_dir_out);
void get_sliders_pos(sliders_position_t *p_sliders_position_out);

#endif /* JOYSTICK_H_ */
