#ifndef CL_GAME_PLACEMENT_SCENE_H
#define CL_GAME_PLACEMENT_SCENE_H
#include "zxcurses/error.h"
#include "game/settings_scene.h"
#include "game/battlefield.h"

TUIError start_placement_screen(GameSettings settings, BattleField* field);

#endif
