#ifndef CL_GAME_SETTINGS_SCENE_H
#define CL_GAME_SETTINGS_SCENE_H

#include "zxcurses/terminal.h"
#include "zxcurses/error.h"

typedef struct {
    TermSizeType width;
    TermSizeType height;
    TermSizeType single_ship_count;
    TermSizeType duo_ship_count;
    TermSizeType triple_ship_count;
    TermSizeType quadriple_ship_count;
} GameSettings;

TUIError start_settings_screen(GameSettings* settings, char** error_msg);

#endif
