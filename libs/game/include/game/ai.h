#ifndef CL_GAME_AI_H
#define CL_GAME_AI_H
#include "game/battlefield.h"
#include "zxcurses/types.h"

typedef struct {
    TermSizeType x;
    TermSizeType y;
    bool result;
    bool axis_horizontal;
    int axis_sign;
    bool found_correct_axis;
} LastShot;

typedef struct {
    LastShot last_shot;
    TermSizeType last_detected_ship_x;
    TermSizeType last_detected_ship_y;
    bool is_searching;

    BattleField* bf;
    bool cant_find_valid_shot;
} AIMemory;

typedef struct {
    TermSizeType x;
    TermSizeType y;
} AIShot;

AIShot ai_get_next_shot(AIMemory* memory);
void ai_set_shot_result(AIMemory* memory, bool success, bool kill);

#endif
