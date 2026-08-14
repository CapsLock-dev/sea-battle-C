#ifndef CL_MAP_MAP_H
#define CL_MAP_MAP_H
#include <stddef.h>
#include "map/errors.h"
#include "map/internal/entry.h"

typedef struct Map Map;

Map* map_init();
void map_free(Map* m);

MapEC map_insert(Map* m, Coord key, int value);
MapEC map_delete(Map* m, Coord key);
MapEC map_find(Map* m, Coord key, int** out_value);

#endif
