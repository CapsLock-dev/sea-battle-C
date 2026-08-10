#include "zxcurses/menu.h"
#include <stdlib.h>

void menu_render(Menu* menu) {
    if (menu == NULL) return;    
    for (size_t i=0; i<menu->count; ++i) {
        if(menu->selected == i) {
            panel_draw_text(menu->panel, 0, (TermSizeType)i, ">");
        } else {
            panel_draw_text(menu->panel, 0, (TermSizeType)i, "|");
        }
        panel_draw_text(menu->panel, 1, (TermSizeType)i, menu->items[i]);
    }
}

void menu_move_selection(Menu* menu, int direction) {
    if (menu == NULL || direction == 0 || menu->count == 0) return;    
    if (direction < 0) {
        menu->selected = (menu->selected + 1) % (menu->count);
    } else if (direction > 0){
        if (menu->selected == 0) {
            menu->selected = menu->count-1;
        } else {
            --menu->selected;
        }
    }
    
}
