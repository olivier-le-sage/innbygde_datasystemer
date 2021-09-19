/*
 * oled.h -- OLED display driver
 *
 * Created: 19.09.2021 13:49:15
 *  Author: oliviel
 */ 


#ifndef OLED_H_
#define OLED_H_

#include <stdint.h>
#include <stdbool.h>

bool oled_init(void);
void oled_reset(void);
void oled_home(void);
void oled_goto_line(uint8_t line);
void oled_goto_column(uint8_t column);
void oled_clear_line(uint8_t line);
void oled_pos(uint8_t row, uint8_t column);
void oled_print(char *output_char);

#endif /* OLED_H_ */