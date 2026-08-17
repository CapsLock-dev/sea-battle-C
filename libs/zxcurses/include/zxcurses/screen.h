#ifndef CL_ZXCURSES_SCREEN_H
#define CL_ZXCURSES_SCREEN_H
#include "zxcurses/color.h"
#include "zxcurses/error.h"
#include "zxcurses/terminal.h"

TUIError create_screen_buffer(TermSizeType width, TermSizeType height);
TUIError resize_screen_buffer(TermSizeType width, TermSizeType height);
void clear_screen_buffer();
void print_screen_buffer();

TUIError draw_text(TermSizeType x, TermSizeType y, char* text, Color fg,
                   Color bg);
TUIError set_cell(TermSizeType x, TermSizeType y, char ch, Color fg, Color bg);

void set_color(TermSizeType x, TermSizeType y, Color fg, Color bg);
void fill_color(TermSizeType x1, TermSizeType y1, TermSizeType x2,
                TermSizeType y2, Color fg, Color bg);

#endif
