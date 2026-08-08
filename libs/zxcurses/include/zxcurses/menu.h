#ifndef CL_ZXCURSES_MENU_H
#define CL_ZXCURSES_MENU_H
#include "zxcurses/panel.h"

typedef struct {
    Panel panel;
    const char** items;
    size_t count;
    size_t selected;
} Menu;

void menu_render(Menu* menu);
void menu_move_selection(Menu* menu, int direction); 

#endif
