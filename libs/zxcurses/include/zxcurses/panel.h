#ifndef CL_ZXCURSES_PANEL_H
#define CL_ZXCURSES_PANEL_H
#include "zxcurses/terminal.h"
#include "zxcurses/color.h"

typedef struct {
    TermSizeType x, y;
    TermSizeType width, height;
} Panel;

void panel_put_char(Panel panel, TermSizeType lx, TermSizeType ly, char ch, Color fg, Color bg);
void panel_draw_text(Panel panel, TermSizeType lx, TermSizeType ly, const char* text, Color fg, Color bg);
void panel_draw_box(Panel panel, Color fg, Color bg);
void panel_fill(Panel panel, char ch, Color fg, Color bg);

#endif
