#include "game/battlefield.h"

#include <stdlib.h>

static bool can_be_placed(BattleField* bf, TermSizeType x, TermSizeType y,
                          bool is_horizontal, ShipType type) {
    TermSizeType width = bf->width;
    TermSizeType height = bf->height;
    TermSizeType size = (TermSizeType)type + 1;
    if (x + size * is_horizontal > width ||
        y + size * (!is_horizontal) > height)
        return false;
    for (TermSizeType i = 0; i < size; ++i) {
        TermSizeType cx = x + i * is_horizontal;
        TermSizeType cy = y + i * (!is_horizontal);
        if (bf_get_cell(bf, cx, cy) != CELL_TYPE_EMPTY) return false;
    }
    return true;
}

static void mark_radius(BattleField* bf, TermSizeType x, TermSizeType y,
                        bool is_horizontal, ShipType type) {
    TermSizeType size = (TermSizeType)type + 1;
    TermSizeType x0 = (x > 0) ? x - 1 : 0;
    TermSizeType y0 = (y > 0) ? y - 1 : 0;
    TermSizeType x1 = is_horizontal ? x + size : x + 1;
    TermSizeType y1 = is_horizontal ? y + 1 : y + size;
    x1 = (x1 < bf->width) ? x1 + 1 : bf->width;
    y1 = (y1 < bf->height) ? y1 + 1 : bf->height;

    for (TermSizeType cy = y0; cy < y1; ++cy) {
        for (TermSizeType cx = x0; cx < x1; ++cx) {
            bool on_ship = is_horizontal
                               ? (cy == y && cx >= x && cx < x + size)
                               : (cx == x && cy >= y && cy < y + size);
            if (on_ship) continue;

            int* existing = NULL;
            if (map_find(bf->field, (Coord){.x = cx, .y = cy}, &existing) ==
                MAP_ERR_Ok)
                continue;
            map_insert(bf->field, (Coord){.x = cx, .y = cy}, 1);
        }
    }
}

static Ship* get_ship(BattleField* bf, int id) {
    int total_ships = bf->id_counter - 2;
    for (int i = 0; i < total_ships; ++i) {
        if (bf->ships[i].id == id) return &bf->ships[i];
    }
    return NULL;
}

static void sunk_ship(BattleField* bf, Ship* ship) {
    TermSizeType x = ship->x;
    TermSizeType y = ship->y;
    bool is_horizontal = ship->is_horizontal;
    TermSizeType size = (TermSizeType)ship->max_size;

    TermSizeType x0 = (x > 0) ? x - 1 : 0;
    TermSizeType y0 = (y > 0) ? y - 1 : 0;
    TermSizeType x1 = is_horizontal ? x + size : x + 1;
    TermSizeType y1 = is_horizontal ? y + 1 : y + size;
    x1 = (x1 < bf->width) ? x1 + 1 : bf->width;
    y1 = (y1 < bf->height) ? y1 + 1 : bf->height;

    for (TermSizeType cy = y0; cy < y1; ++cy) {
        for (TermSizeType cx = x0; cx < x1; ++cx) {
            bool on_ship = is_horizontal
                               ? (cy == y && cx >= x && cx < x + size)
                               : (cx == x && cy >= y && cy < y + size);
            if (on_ship) continue;

            int* existing = NULL;
            MapEC ec =
                map_find(bf->field, (Coord){.x = cx, .y = cy}, &existing);
            if (ec == MAP_ERR_Ok) {
                if (*existing == 1) *existing = 0;
            } else {
                map_insert(bf->field, (Coord){.x = cx, .y = cy}, 0);
            }
        }
    }
}

BattleField* bf_init(GameSettings* settings) {
    BattleField* bf = malloc(sizeof(BattleField));
    if (bf == NULL) return NULL;
    bf->field = map_init();
    if (bf->field == NULL) {
        free(bf);
        return NULL;
    }
    bf->width = settings->width;
    bf->height = settings->height;
    unsigned int all_ship_count =
        settings->single_ship_count + settings->duo_ship_count +
        settings->triple_ship_count + settings->quadriple_ship_count;
    bf->ships = calloc(all_ship_count, sizeof(Ship));
    bf->ships_left = 0;
    bf->id_counter = 2;
    if (bf->ships == NULL) {
        free(bf->field);
        free(bf);
        return NULL;
    }

    return bf;
}

BattleField* bf_init_random(GameSettings* settings) {
    BattleField* bf = bf_init(settings);
    if (bf == NULL) return NULL;

    ShipType types[4] = {SHIP_TYPE_QUADRIPLE, SHIP_TYPE_TRIPLE,
                         SHIP_TYPE_DOUBLE, SHIP_TYPE_SIGNLE};
    unsigned int counts[4] = {
        settings->quadriple_ship_count,
        settings->triple_ship_count,
        settings->duo_ship_count,
        settings->single_ship_count,
    };

    const int max_attempts_per_ship = 10000;

    for (int t = 0; t < 4; ++t) {
        for (unsigned int i = 0; i < counts[t]; ++i) {
            GameEC ec = GAME_EC_CantPlaceHere;
            int attempts = 0;
            while (ec != GAME_EC_Ok) {
                if (attempts++ >= max_attempts_per_ship) {
                    bf_free(bf);
                    return NULL;
                }
                bool is_horizontal = rand() % 2;
                TermSizeType x = (TermSizeType)(rand() % bf->width);
                TermSizeType y = (TermSizeType)(rand() % bf->height);
                ec = bf_place_ship(bf, x, y, is_horizontal, types[t]);
                if (ec == GAME_EC_InternalMapError ||
                    ec == GAME_EC_AllocationError) {
                    bf_free(bf);
                    return NULL;
                }
            }
        }
    }

    return bf;
}

GameEC bf_place_ship(BattleField* bf, TermSizeType x, TermSizeType y,
                     bool is_horizontal, ShipType type) {
    if (!can_be_placed(bf, x, y, is_horizontal, type))
        return GAME_EC_CantPlaceHere;
    Ship ship = {.id = bf->id_counter,
                 .current_size = type + 1,
                 .max_size = type + 1,
                 .x = x,
                 .y = y,
                 .is_horizontal = is_horizontal};
    MapEC ec = MAP_ERR_Ok;
    for (TermSizeType i = 0; i < type + 1; ++i) {
        TermSizeType cx = x + i * is_horizontal;
        TermSizeType cy = y + i * (!is_horizontal);
        ec = map_insert(bf->field, (Coord){.x = cx, .y = cy}, ship.id);
        if (ec != MAP_ERR_Ok) return GAME_EC_InternalMapError;
    }
    mark_radius(bf, x, y, is_horizontal, type);
    bf->ships[bf->ships_left++] = ship;
    ++bf->id_counter;

    return GAME_EC_Ok;
}

GameEC bf_shot(BattleField* bf, TermSizeType x, TermSizeType y, bool* is_hit,
               Ship** sunken_ship) {
    if (x >= bf->width || y >= bf->height) return GAME_EC_CantPlaceHere;
    *is_hit = false;

    int* value = NULL;
    MapEC ec = map_find(bf->field, (Coord){.x = x, .y = y}, &value);
    if (ec == MAP_ERR_KeyDoesntExists) {
        // Plain water, never touched before: record it as a miss.
        ec = map_insert(bf->field, (Coord){.x = x, .y = y}, 0);
        if (ec != MAP_ERR_Ok) return GAME_EC_InternalMapError;
        return GAME_EC_Ok;
    }

    if (*value == 1) {
        // First shot on a ship's radius cell: reveal it as a miss.
        *value = 0;
        return GAME_EC_Ok;
    }
    if (*value < 1) {
        // Already shot here before (previous miss, or an already-hit
        // ship cell which is now negative).
        return GAME_EC_CantPlaceHere;
    }

    // *value > 1: an unshot ship cell -- this is a hit.
    int ship_id = *value;
    *value = -ship_id;
    *is_hit = true;

    Ship* ship = get_ship(bf, ship_id);
    if (ship == NULL) return GAME_EC_InternalMapError;

    if (ship->current_size > 0) --ship->current_size;
    if (ship->current_size == 0) {
        sunk_ship(bf, ship);
        if (bf->ships_left > 0) --bf->ships_left;
        *sunken_ship = ship;
    }

    return GAME_EC_Ok;
}

bool is_end(BattleField* field) { return field->ships_left == 0; }

CellType bf_get_cell(BattleField* bf, TermSizeType x, TermSizeType y) {
    Map* m = bf->field;
    int* value = NULL;
    MapEC ec = map_find(m, (Coord){.x = x, .y = y}, &value);
    if (ec == MAP_ERR_KeyDoesntExists) return CELL_TYPE_EMPTY;
    if (*value > 1) {
        return CELL_TYPE_SHIP;
    } else if (*value < 0) {
        return CELL_TYPE_HIT;
    } else if (*value == 1) {
        return CELL_TYPE_RADIUS;
    } else {
        return CELL_TYPE_MISS;
    }
}

void bf_free(BattleField* bf) {
    if (bf == NULL) return;
    map_free(bf->field);
    free(bf->ships);
    free(bf);
}
