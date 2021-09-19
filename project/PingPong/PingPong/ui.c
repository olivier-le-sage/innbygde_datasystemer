/*
 * ui.c
 *
 * Created: 19.09.2021 14:54:06
 *  Author: oliviel
 */ 

#include "ui.h"
#include "oled.h"
#include "ping_pong.h"
#include <stdlib.h>

static ui_submenu_t m_final_menu = {
	.num_submenu_options = 1,
	.submenu_options[0] = "(1) :(",
};

static ui_submenu_t m_next_menu = {
	.num_submenu_options = 2,
	.submenu_options[0]  = "(1) Welcome to the submenu",
	.submenu_options[1]  = "(2) Exit",
	.next[1]             = &m_final_menu,
};

static ui_submenu_t m_main_menu = {
	.num_submenu_options = 5,
	.submenu_options[0]  = "(1) Play game",
	.submenu_options[1]  = "(2) Run animation",
	.submenu_options[2]  = "(3) Play with fire",
	.submenu_options[3]  = "(4) Launch the nukes",
	.submenu_options[4]  = "(5) Exit",
	.next[3]             = &m_next_menu,
};

static uint8_t m_current_selection;
static ui_submenu_t *mp_current_menu;

void m_update_display(void)
{
	/* Print out current menu to OLED display */
	// TODO
	//oled_goto_line(...)
	//oled_clear_line(...)
	//oled_print(...)
}


bool ui_init(void)
{
	assert(NUMELTS(m_main_menu.submenu_options) <= m_main_menu.num_submenu_options);
	assert(m_main_menu.num_submenu_options <= MAX_SUBMENU_OPTIONS);

	m_current_selection = 0;	
	mp_current_menu = &m_main_menu;

	return true;
}

void m_ui_go_up(void)
{
	if (m_current_selection < mp_current_menu->num_submenu_options)
	{
		m_current_selection++;
	}
}

void m_ui_go_down(void)
{
	if (m_current_selection > mp_current_menu->num_submenu_options)
	{
		m_current_selection--;
	}
}

void m_ui_enter(void)
{
	if (mp_current_menu->next)
	{
		ui_submenu_t *p_next_submenu = mp_current_menu->next[m_current_selection];

		if (p_next_submenu)
		{
			mp_current_menu = p_next_submenu;
		}
	}
}

void ui_issue_cmd(ui_cmd_t cmd)
{
	switch(cmd)
	{
		case(UI_DO_NOTHING):
			break;
		case(UI_SELECT_DOWN):
			m_ui_go_down();
			break;
		case(UI_SELECT_UP):
			m_ui_go_up();
			break;
		case(UI_ENTER_SUBMENU):
			m_ui_enter();
			break;
		default:
			assert(false);
			break;
	}
	
	m_update_display();
}