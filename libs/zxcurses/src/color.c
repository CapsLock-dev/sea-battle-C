#include "zxcurses/color.h"

int color_bg_ansi_code(Color color) {
    if (COLOR_DEFAULT == color) return 49;
    if (color >= COLOR_BLACK && color <= COLOR_WHITE)
        return (int)(40 + color - COLOR_BLACK);
    if (color >= COLOR_BRIGHT_BLACK && color <= COLOR_BRIGHT_WHITE)
        return (int)(100 + color - COLOR_BRIGHT_BLACK);
    return 47;
}

int color_fg_ansi_code(Color color) {
    if (COLOR_DEFAULT == color) return 39;
    if (color >= COLOR_BLACK && color <= COLOR_WHITE)
        return (int)(30 + color - COLOR_BLACK);
    if (color >= COLOR_BRIGHT_BLACK && color <= COLOR_BRIGHT_WHITE)
        return (int)(90 + color - COLOR_BRIGHT_BLACK);
    return 30;
}
