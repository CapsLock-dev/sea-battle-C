#ifndef CL_GAME_BATTLE_SCENE_H
#define CL_GAME_BATTLE_SCENE_H
#include "game/battlefield.h"
#include "game/error.h"

GameEC start_battle_scene(BattleField* player1_field,
                          BattleField* player2_field, bool enable_ai);

#endif
