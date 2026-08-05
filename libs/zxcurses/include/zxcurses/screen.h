#ifndef CL_ZXCURSES_SCREEN_H
#define CL_ZXCURSES_SCREEN_H
#include "zxcurses/terminal.h"

bool create_screen_buffer(TermSizeType width, TermSizeType height);
bool resize_screen_buffer(TermSizeType width, TermSizeType height);
void print_screen_buffer();

void draw_text(TermSizeType x, TermSizeType y, char* text);

#endif
