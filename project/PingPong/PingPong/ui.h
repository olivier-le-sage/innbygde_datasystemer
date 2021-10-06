/*
 * ui.h
 *
 * Created: 19.09.2021 14:54:16
 *  Author: oliviel
 */ 


#ifndef UI_H_
#define UI_H_

#include <stdbool.h>
#include <stdint.h>

#define MAX_SUBMENU_OPTIONS (5)  /* Max number of options in any menu */
#define MAX_MENU_LINE_SIZE  (32) /* Max number of characters in a row of a menu */

typedef struct ui_submenu_t ui_submenu_t;

struct ui_submenu_t
{
	uint8_t num_submenu_options;
	const char * submenu_options[MAX_SUBMENU_OPTIONS];
	ui_submenu_t* next[MAX_SUBMENU_OPTIONS];
};

typedef enum
{
	UI_DO_NOTHING = 0,
	UI_SELECT_DOWN,
	UI_SELECT_UP,
	UI_ENTER_SUBMENU,
} ui_cmd_t;

bool ui_init(void);
void ui_issue_cmd(ui_cmd_t cmd);

#endif /* UI_H_ */