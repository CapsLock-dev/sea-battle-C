#include "datatype/datatype.h"

#include <stdlib.h>

#include "cltest/cltest.h"

CompareResult cmp(const DataEnvelope* a, const DataEnvelope* b) {
    if (a->type->id != 1 || b->type->id != 1) return CMP_WRONG;
    int ai = *(int*)a->data;
    int bi = *(int*)b->data;
    if (ai > bi) {
        return CMP_MORE;
    } else if (ai < bi) {
        return CMP_LESS;
    }
    return CMP_EQUAL;
}

size_t hash(const DataEnvelope* a) {
    if (a->type->id != 1) return 0;
    int ai = *(int*)a->data;
    return (size_t)(ai * ai) * 978;
}

TEST_F(EnvelopeInt) {
    int a = 10;
    int b = 12;
    DataEnvelope* env_a = NULL;
    DataEnvelope* env_b = NULL;

    const DataType* type = datatype_create(1, cmp, hash, sizeof(int));
    EXPECT_TRUE(type != NULL);

    env_a = envelope_create(type, &a);
    env_b = envelope_create(type, &b);
    EXPECT_TRUE(env_a != NULL);
    EXPECT_TRUE(env_b != NULL);

    size_t hash1 = type->hash(env_a);
    size_t hash2 = type->hash(env_b);
    EXPECT_NEQ_NUM(hash1, hash2);

    EXPECT_EQ_NUM(type->cmp(env_a, env_b), CMP_LESS);
    EXPECT_EQ_NUM(type->cmp(env_b, env_a), CMP_MORE);
    EXPECT_EQ_NUM(type->cmp(env_a, env_a), CMP_EQUAL);
    EXPECT_EQ_NUM(type->cmp(env_b, env_b), CMP_EQUAL);

    TEAR_DOWN(datatype_free(type); envelope_free(env_a); envelope_free(env_b);)
}

typedef struct Coord {
    int x;
    int y;
} Coord;

CompareResult cmp_coord(const DataEnvelope* a, const DataEnvelope* b) {
    if (a->type->id != 2 || b->type->id != 2) return CMP_WRONG;
    Coord ai = *(Coord*)a->data;
    Coord bi = *(Coord*)b->data;

    if (ai.y > bi.y) return CMP_MORE;
    if (ai.y < bi.y) return CMP_LESS;
    if (ai.x > bi.x) return CMP_MORE;
    if (ai.x < bi.x) return CMP_LESS;

    return CMP_EQUAL;
}

size_t hash_coord(const DataEnvelope* a) {
    if (a->type->id != 2) return CMP_WRONG;
    Coord ai = *(Coord*)a->data;
    return (size_t)((ai.x + ai.y) * (ai.x + ai.y) * 937);
}

TEST_F(EnvelopeStruct) {
    Coord a = {.x = 10, .y = 1};
    Coord b = {.x = 12, .y = 1};
    Coord c = {.x = 1, .y = 2};
    int d = 10;
    DataEnvelope* env_a = NULL;
    DataEnvelope* env_b = NULL;
    DataEnvelope* env_c = NULL;
    DataEnvelope* env_d = NULL;

    const DataType* type =
        datatype_create(2, cmp_coord, hash_coord, sizeof(Coord));
    const DataType* type_int = datatype_create(1, cmp, hash, sizeof(int));
    EXPECT_TRUE(type != NULL);
    EXPECT_TRUE(type_int != NULL);

    env_a = envelope_create(type, &a);
    env_b = envelope_create(type, &b);
    env_c = envelope_create(type, &c);
    env_d = envelope_create(type_int, &d);
    EXPECT_TRUE(env_a != NULL);
    EXPECT_TRUE(env_b != NULL);
    EXPECT_TRUE(env_c != NULL);
    EXPECT_TRUE(env_d != NULL);

    size_t hash1 = type->hash(env_a);
    size_t hash2 = type->hash(env_b);
    size_t hash3 = type_int->hash(env_c);
    size_t hash4 = type->hash(env_d);
    EXPECT_NEQ_NUM(hash1, hash2);
    EXPECT_EQ_NUM(hash3, 0);
    EXPECT_EQ_NUM(hash4, 0);

    EXPECT_EQ_NUM(type->cmp(env_a, env_b), CMP_LESS);
    EXPECT_EQ_NUM(type->cmp(env_b, env_a), CMP_MORE);
    EXPECT_EQ_NUM(type->cmp(env_a, env_a), CMP_EQUAL);
    EXPECT_EQ_NUM(type->cmp(env_b, env_b), CMP_EQUAL);
    EXPECT_EQ_NUM(type->cmp(env_c, env_b), CMP_MORE);

    TEAR_DOWN(datatype_free(type); datatype_free(type_int);
              envelope_free(env_a); envelope_free(env_b); envelope_free(env_c);
              envelope_free(env_d);)
}
