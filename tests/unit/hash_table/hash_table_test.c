#include "cltest/cltest.h"
#include "test-utils/envelope_destructor.h"
#include "hash_table/hash_table.h"
#include <stdlib.h>

typedef struct Coord {
    int x;
    int y;
} Coord;

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

size_t hash_coord(const DataEnvelope* a) {
    if (a->type->id != 1) return CMP_WRONG;
    //Coord ai = *(Coord*)a->data;
    return 1;
}

#define envelope(type, data) s_envelope_init(envelope_create(type, data))

TEST_F(HashTableInit) {
    HashTable* ht = NULL;
    const DataType* coord_type = datatype_create(1, cmp_coord, hash_coord, sizeof(Coord));
    const DataType* int_type = datatype_create(2, NULL, NULL, sizeof(int));
    EXPECT_TRUE(coord_type != NULL);
    EXPECT_TRUE(int_type != NULL);

    ht = hash_table_init(100, coord_type, int_type);
    EXPECT_TRUE(ht != NULL);

    TEAR_DOWN(
        datatype_free(coord_type);
        datatype_free(int_type);
        hash_table_free(ht);
    )
}

TEST_F(HashTableInsert) {
    Coord a = {.x = 10, .y = 1};
    Coord b = {.x = 12, .y = 1};
    Coord c = {.x = 1, .y = 2};
    int a1 = 1;
    HashTable* ht = NULL;
    const DataType* coord_type = datatype_create(1, cmp_coord, hash_coord, sizeof(Coord));
    const DataType* int_type = datatype_create(2, NULL, NULL, sizeof(int));
    EXPECT_TRUE(coord_type != NULL);
    EXPECT_TRUE(int_type != NULL);

    ht = hash_table_init(2, coord_type, int_type);
    EXPECT_TRUE(ht != NULL);

    DataEnvelope* env_a = envelope(coord_type, &a);
    DataEnvelope* env_b = envelope(coord_type, &b);
    DataEnvelope* env_c = envelope(coord_type, &c);
    DataEnvelope* val_a1 = envelope(int_type, &a1);

    HashTableEC ec = hash_table_insert(ht, env_a, val_a1);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);
    ec = hash_table_insert(ht, env_a, val_a1);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_KeyExists);
    ec = hash_table_insert(ht, env_b, val_a1);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);

    ec = hash_table_insert(ht, env_c, val_a1);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Full);
    ec = hash_table_insert(ht, NULL, NULL);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_IsNull);

    TEAR_DOWN(
        s_envelope_free_all();
        datatype_free(coord_type);
        datatype_free(int_type);
        hash_table_free(ht);
    )
}

TEST_F(HashTableDelete) {
    const DataType* coord_type = datatype_create(1, cmp_coord, hash_coord, sizeof(Coord));
    const DataType* int_type = datatype_create(2, NULL, NULL, sizeof(int));
    HashTable* ht = hash_table_init(20, coord_type, int_type);

    HashTableEC ec = HASH_TABLE_EC_Ok;
    for (size_t i=0; i<ht->max_size; ++i) {
        Coord key = {.x = (int)i, .y = (int)i};
        int val = (int)i;
        DataEnvelope* key_temp = envelope(coord_type, &key);
        DataEnvelope* val_temp = envelope(int_type, &val);
        ec = hash_table_insert(ht, key_temp, val_temp);
        EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);
    }

    Coord key = {.x = 1, .y = 1};
    DataEnvelope* key_env = envelope(coord_type, &key);
    ec = hash_table_delete(ht, key_env);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);
    ec = hash_table_delete(ht, key_env);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_KeyDoesntExists);
    int val = 10;
    DataEnvelope* val_env = envelope(int_type, &val);
    ec = hash_table_insert(ht, key_env, val_env);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);

    for (size_t i=0; i<ht->max_size; ++i) {
        Coord key1 = {.x = (int)i, .y = (int)i};
        DataEnvelope* key_temp = envelope(coord_type, &key1);
        ec = hash_table_delete(ht, key_temp);
        EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);
    }

    TEAR_DOWN(
        s_envelope_free_all();
        datatype_free(coord_type);
        datatype_free(int_type);
        hash_table_free(ht);
    )
}

TEST_F(HashTableFind) {
    const DataType* coord_type = datatype_create(1, cmp_coord, hash_coord, sizeof(Coord));
    const DataType* int_type = datatype_create(2, NULL, NULL, sizeof(int));
    HashTable* ht = hash_table_init(20, coord_type, int_type);
    HashTableEC ec = HASH_TABLE_EC_Ok;

    Coord k1 = {.x=1, .y=1};
    //int v1 = 1;
    DataEnvelope* key1 = envelope(coord_type, &k1);
    DataEnvelope* val1 = NULL;
    ec = hash_table_find(ht, key1, &val1);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_KeyDoesntExists);

    for (size_t i=0; i<ht->max_size; ++i) {
        Coord key = {.x = (int)i, .y = (int)i};
        int val = (int)i;
        DataEnvelope* key_temp = envelope(coord_type, &key);
        DataEnvelope* val_temp = envelope(int_type, &val);
        ec = hash_table_insert(ht, key_temp, val_temp);
        EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);
    }

    ec = hash_table_find(ht, key1, &val1);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);
    EXPECT_EQ_NUM(*(int*)val1->data, 1);
    ec = hash_table_delete(ht, key1);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);
    ec = hash_table_find(ht, key1, &val1);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_KeyDoesntExists);
    Coord k2 = {.x=2, .y=2};
    DataEnvelope* key2 = envelope(coord_type, &k2);
    ec = hash_table_find(ht, key2, &val1);
    EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);
    EXPECT_EQ_NUM(*(int*)val1->data, 2);

    TEAR_DOWN(
        s_envelope_free_all();
        datatype_free(coord_type);
        datatype_free(int_type);
        hash_table_free(ht);
    )
}
