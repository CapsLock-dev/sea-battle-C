#ifndef CL_ZXCURSES_PANEL_H
#define CL_ZXCURSES_PANEL_H
#include "zxcurses/terminal.h"

typedef struct {
    TermSizeType x, y;
    TermSizeType width, height;
} Panel;

void panel_put_char(Panel panel, TermSizeType lx, TermSizeType ly, char ch);
void panel_draw_text(Panel panel, TermSizeType lx, TermSizeType ly, const char* text);
void panel_draw_box(Panel panel);
void panel_fill(Panel panel, char ch);

#endif
