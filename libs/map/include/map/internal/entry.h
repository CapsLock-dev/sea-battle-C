#ifndef CL_MAP_INTERNAL_ENTRY_H
#define CL_MAP_INTERNAL_ENTRY_H
#include "datatype/datatype.h"

typedef struct {
    int x;
    int y;
} Coord;

CompareResult cmp_coord(const DataEnvelope* a, const DataEnvelope* b);
CompareResult cmp_int(const DataEnvelope* a, const DataEnvelope* b);
size_t hash_coord(const DataEnvelope* a);

#endif
