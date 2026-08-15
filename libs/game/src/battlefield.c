#include "game/battlefield.h"
#include <stdlib.h>

BattleField* bf_init(GameSettings* settings) {
    BattleField* bf = malloc(sizeof(BattleField));
    if (bf == NULL) return NULL;
    bf->field = map_init();
    if (bf->field == NULL) {free(bf); return NULL;}
    bf->width = settings->width;
    bf->height = settings->height;
    unsigned int all_ship_count = settings->single_ship_count+settings->duo_ship_count+settings->triple_ship_count+settings->quadriple_ship_count;
    bf->ships = calloc(all_ship_count, sizeof(Ship));
    bf->ships_left = all_ship_count;
    if (bf->ships == NULL) {free(bf->field); free(bf); return NULL;}

    return bf;
}

bool bf_place_ship(BattleField* bf, TermSizeType x, TermSizeType y, bool is_horizontal, ShipType type) {
    TermSizeType ship_size = (TermSizeType)type+1;
    TermSizeType x_len = 1;
    TermSizeType y_len = 1;
    if (is_horizontal) x_len = ship_size;
    if (!is_horizontal) y_len = ship_size;
    if (x+x_len > bf->width || y+y_len > bf->height) return false;

    return true;
}

bool bf_shot(BattleField* bf, TermSizeType x, TermSizeType y) {
}

CellType bf_get_cell(BattleField* bf, TermSizeType x, TermSizeType y) {
    Map* m = bf->field;
    int* value = 0;
    MapEC ec = map_find(m, (Coord){.x=x,.y=y}, &value);
    if (ec == MAP_ERR_KeyDoesntExists) return CELL_TYPE_EMPTY;
    if (ec != MAP_ERR_Ok) return CELL_TYPE_ERROR;
}

void bf_free(BattleField* bf) {
    if (bf == NULL) return;
    map_free(bf->field);
    free(bf->ships);
    free(bf);
}
