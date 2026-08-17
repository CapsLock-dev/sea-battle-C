#include "map/internal/entry.h"

CompareResult cmp_coord(const DataEnvelope* a, const DataEnvelope* b) {
    if (a->type->id != 1 || b->type->id != 1) return CMP_WRONG;
    Coord ai = *(Coord*)a->data;
    Coord bi = *(Coord*)b->data;

    if (ai.y > bi.y) return CMP_MORE;
    if (ai.x > bi.x) return CMP_MORE;

    if (ai.y < bi.y) return CMP_LESS;
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
    // Coord ai = *(Coord*)a->data;
    return 1;
}
