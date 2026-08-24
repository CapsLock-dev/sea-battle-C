#include "map/internal/entry.h"
#include <stdint.h>
#include <stdlib.h>

CompareResult cmp_coord(const DataEnvelope* a, const DataEnvelope* b) {
    if (a->type->id != 1 || b->type->id != 1) return CMP_WRONG;
    Coord ai = *(Coord*)a->data;
    Coord bi = *(Coord*)b->data;

    if (ai.y > bi.y) return CMP_MORE;
    if (ai.y < bi.y) return CMP_LESS;
    if (ai.x > bi.x) return CMP_MORE;
    if (ai.x < bi.x) return CMP_LESS;

    return CMP_EQUAL;
}

CompareResult cmp_int(const DataEnvelope* a, const DataEnvelope* b) {
    if (a->type->id != 2 || b->type->id != 2) return CMP_WRONG;
    int ai = *(int*)a->data;
    int bi = *(int*)b->data;

    if (ai > bi) return CMP_MORE;
    if (ai < bi) return CMP_LESS;

    return CMP_EQUAL;
}

size_t hash_coord(const DataEnvelope* a) {
    if (a->type->id != 1) return CMP_WRONG;
    Coord ai = *(Coord*)a->data;
    uint64_t x = (uint64_t)(ai.x);
    uint64_t y = (uint64_t)(ai.y);
    uint64_t n1 = 1312931939139129319;
    uint64_t n2 = 3929319231939757553;
    uint64_t hash = (y << 32) | x;
    hash *= n1;
    hash ^= (hash >> 32);
    hash *= n2;
    hash ^= (hash >> 32);
    return hash;
}
