#ifndef CL_GAME_BATTLEFIELD_H
#define CL_GAME_BATTLEFIELD_H
#include "map/map.h"
#include "zxcurses/types.h"
#include "game/settings_scene.h"
#include "game/error.h"

typedef struct {
    int id;
    size_t current_size;
    size_t max_size;
    TermSizeType x;
    TermSizeType y;
    bool is_horizontal;
} Ship;

typedef struct {
    Map* field; // value = 0 miss, value = 1 ship radius, value > 1 ship_id, value < 0 sunken ship_id, not found = water
    Ship* ships;
    TermSizeType width;
    TermSizeType height;
    unsigned int ships_left;
    int id_counter;
} BattleField;

typedef enum {
    SHIP_TYPE_SIGNLE,
    SHIP_TYPE_DOUBLE,
    SHIP_TYPE_TRIPLE,
    SHIP_TYPE_QUADRIPLE,
} ShipType;

typedef enum {
    CELL_TYPE_EMPTY,
    CELL_TYPE_SHIP,
    CELL_TYPE_RADIUS,
    CELL_TYPE_MISS,
    CELL_TYPE_HIT,
} CellType;

BattleField* bf_init(GameSettings* settings);
BattleField* bf_init_random(GameSettings* settings);

GameEC bf_place_ship(BattleField* bf, TermSizeType x, TermSizeType y, bool is_horizontal, ShipType type);
GameEC bf_shot(BattleField* bf, TermSizeType x, TermSizeType y, bool* is_hit, Ship** sunken_ship);
bool is_end(BattleField* field);

CellType bf_get_cell(BattleField* bf, TermSizeType x, TermSizeType y);
void bf_free(BattleField* bf);

#endif
