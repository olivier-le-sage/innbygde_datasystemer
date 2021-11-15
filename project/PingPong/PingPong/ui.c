/*
 * ui.c
 *
 * Created: 19.09.2021 14:54:06
 *  Author: oliviel
 */ 

#include "ui.h"
#include "oled.h"
#include "ping_pong.h"
#include "utils.h"
#include <stdlib.h>

// Max text size: length of prelude + max number length + terminator
#define M_SCORE_TEXT "Score: "
#define M_SCORE_TEXT_MAX_LEN (sizeof(M_SCORE_TEXT) + sizeof(STRINGIFY(UINT32_MAX)) - 1)

static char m_score_text[M_SCORE_TEXT_MAX_LEN];

static ui_submenu_t m_game_screen  = {
	.disable_selection = true,
	.num_submenu_options = 1,
	.submenu_options[0] = m_score_text,
};

static ui_submenu_t m_final_menu = {
	.num_submenu_options = 1,
	.submenu_options[0] = "(1) :(",
};

static ui_submenu_t m_next_menu = {
	.num_submenu_options = 2,
	.submenu_options[0]  = "(1) WELCOME TO THE SUBMENU",
	.submenu_options[1]  = "(2) EXIT",
	.next[1]             = &m_final_menu,
};

static ui_submenu_t m_main_menu = {
	.num_submenu_options = 5,
	.submenu_options[0]  = "(1) PLAY GAME",
	.submenu_options[1]  = "(2) RUN ANIMATION",
	.submenu_options[2]  = "(3) PLAY WITH FIRE",
	.submenu_options[3]  = "(4) LAUNCH THE NUKES",
	.submenu_options[4]  = "(5) EXIT",
	.next[0] 			 = &m_game_screen,
	.next[3]             = &m_next_menu,
};

static uint8_t m_current_selection;
static ui_submenu_t *mp_current_menu;

void m_update_display(void)
{
	oled_reset();

	/* Print out current menu to OLED display -- always assume it has changed for now */
	for (uint8_t i = 0; i < mp_current_menu->num_submenu_options; i++)
	{
		oled_goto_line(i);
		oled_printf(mp_current_menu->submenu_options[i],
				    !mp_current_menu->disable_selection && (i == m_current_selection));
	}
}


bool ui_init(void)
{
	assert(oled_init());
	assert(NUMELTS(m_main_menu.submenu_options) <= m_main_menu.num_submenu_options);
	assert(m_main_menu.num_submenu_options <= MAX_SUBMENU_OPTIONS);

	m_current_selection = 0;
	mp_current_menu = &m_main_menu;

	ui_game_screen_update(0);

	return true;
}

void m_ui_go_up(void)
{
	if (m_current_selection < (mp_current_menu->num_submenu_options - 1))
	{
		m_current_selection++;
	}
}

void m_ui_go_down(void)
{
	if (m_current_selection != 0)
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
		
		/* Reset "cursor" to the top of the menu options */
		m_current_selection = 0;
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

void ui_game_screen_update(uint32_t score)
{
	snprintf(m_score_text, M_SCORE_TEXT_MAX_LEN - 1, M_SCORE_TEXT "%lu", score); 
}