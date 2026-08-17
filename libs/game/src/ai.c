#include "game/ai.h"

#include <stdlib.h>

static AIShot finisher_mode(AIMemory* memory);
static AIShot direction_finder_mode(AIMemory* memory);
static AIShot random_shot(AIMemory* memory);
static bool is_valid_shot(AIMemory* memory, AIShot shot);

AIShot ai_get_next_shot(AIMemory* memory) {
    AIShot shot = {.x = 0, .y = 0};

    LastShot* last_shot = &memory->last_shot;
    if (memory->is_searching) {
        shot = random_shot(memory);
    } else if (last_shot->found_correct_axis) {
        shot = finisher_mode(memory);
    } else {
        shot = direction_finder_mode(memory);
    }

    if (memory->cant_find_valid_shot) {
        shot = random_shot(memory);
        memory->cant_find_valid_shot = false;
    }

    memory->last_shot.x = shot.x;
    memory->last_shot.y = shot.y;
    return shot;
}

void ai_set_shot_result(AIMemory* memory, bool success, bool kill) {
    if (kill) {
        memory->is_searching = true;
        memory->last_shot.found_correct_axis = false;
        return;
    }
    LastShot* last_shot = &memory->last_shot;
    last_shot->result = success;
    if (success && !last_shot->found_correct_axis && !memory->is_searching) {
        last_shot->found_correct_axis = true;
    }
    if (success && memory->is_searching) {
        memory->last_detected_ship_x = last_shot->x;
        memory->last_detected_ship_y = last_shot->y;
        memory->is_searching = false;
    }
}

static AIShot finisher_mode(AIMemory* memory) {
    LastShot* last_shot = &memory->last_shot;
    AIShot shot = {0, 0};
    int dx = last_shot->axis_horizontal * last_shot->axis_sign;
    int dy = (!last_shot->axis_horizontal) * last_shot->axis_sign;
    if (last_shot->result) {
        TermSizeType x = last_shot->x;
        TermSizeType y = last_shot->y;
        shot = (AIShot){x + dx, y + dy};
        if (!is_valid_shot(memory, shot)) {
            dx *= -1;
            dy *= -1;
            x = memory->last_detected_ship_x;
            y = memory->last_detected_ship_y;
            shot = (AIShot){x + dx, y + dy};
            if (!is_valid_shot(memory, shot)) {
                memory->cant_find_valid_shot = true;
            }
        }
    } else {
        dx *= -1;
        dy *= -1;
        TermSizeType x = memory->last_detected_ship_x;
        TermSizeType y = memory->last_detected_ship_y;
        shot = (AIShot){x + dx, y + dy};
        if (!is_valid_shot(memory, shot)) {
            memory->cant_find_valid_shot = true;
        }
    }
    return shot;
}

static AIShot direction_finder_mode(AIMemory* memory) {
    LastShot* last_shot = &memory->last_shot;
    AIShot shot = {.x = 0, .y = 0};
    struct coord_delta {
        int dx;
        int dy;
    };
    bool valid_shot = false;
    struct coord_delta shots[4] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    for (int i = 0; i < 4; ++i) {
        shot.x = (TermSizeType)(memory->last_detected_ship_x + shots[i].dx);
        shot.y = (TermSizeType)(memory->last_detected_ship_y + shots[i].dy);
        valid_shot = is_valid_shot(memory, shot);
        if (valid_shot) {
            last_shot->axis_sign = shots[i].dx + shots[i].dy;
            last_shot->axis_horizontal = (shots[i].dx != 0);
            break;
        }
    }
    if (!valid_shot) {
        memory->cant_find_valid_shot = true;
    }
    return shot;
}

static AIShot random_shot(AIMemory* memory) {
    AIShot shot = {.x = 0, .y = 0};
    int attempts = 100;
    bool valid_shot = false;
    while (!valid_shot && attempts > 0) {
        shot.x = (TermSizeType)(rand() % memory->bf->width);
        shot.y = (TermSizeType)(rand() % memory->bf->height);
        --attempts;
        valid_shot = is_valid_shot(memory, shot);
    }
    if (!valid_shot) {
        memory->cant_find_valid_shot = true;
    }
    return shot;
}

static bool is_valid_shot(AIMemory* memory, AIShot shot) {
    if (shot.x >= memory->bf->width || shot.y >= memory->bf->height)
        return false;
    CellType cell = bf_get_cell(memory->bf, shot.x, shot.y);
    return (cell == CELL_TYPE_EMPTY || cell == CELL_TYPE_SHIP ||
            cell == CELL_TYPE_RADIUS);
}
