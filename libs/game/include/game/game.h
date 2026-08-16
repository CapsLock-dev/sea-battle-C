#ifndef CL_GAME_GAME_H
#define CL_GAME_GAME_H
#include "game/error.h"
#include "game/settings_scene.h"

GameEC start_multiplayer_game(GameSettings settings);
GameEC start_singleplayer_game(GameSettings settings);

#endif
