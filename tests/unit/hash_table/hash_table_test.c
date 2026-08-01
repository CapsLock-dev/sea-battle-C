#include "cltest/cltest.h"
#include "test-utils/envelope_destructor.h"
#include "test-utils/array_tools.h"
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

    ht = hash_table_init(coord_type, int_type);
    EXPECT_TRUE(ht != NULL);

    TEAR_DOWN(
        datatype_free(coord_type);
        datatype_free(int_type);
        hash_table_free(ht);
    )
}

#define TEST_ARRAY_SIZE 500

TEST_F(HashTableInsert) {
    HashTable* ht = NULL;
    const DataType* coord_type = datatype_create(1, cmp_coord, hash_coord, sizeof(Coord));
    const DataType* int_type = datatype_create(2, NULL, NULL, sizeof(int));
    EXPECT_TRUE(coord_type != NULL);
    EXPECT_TRUE(int_type != NULL);

    ht = hash_table_init(coord_type, int_type);
    EXPECT_TRUE(ht != NULL);

    HashTableEC ec = HASH_TABLE_EC_Undefined;
    Coord array[TEST_ARRAY_SIZE] = {};
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) { 
        Coord key = {.x = (int)i, .y = (int)i};
        array[i] = key;
    }
    array_shuffle(array, TEST_ARRAY_SIZE, sizeof(Coord));
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) {
        printf("Key: X-%d Y-%d", array[i].x, array[i].y);
        DataEnvelope* key = envelope(coord_type, &array[i]);
        DataEnvelope* value = envelope(int_type, &i);
        ec = hash_table_insert(ht, key, value);
        EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);
    }

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
    HashTable* ht = hash_table_init(coord_type, int_type);

    HashTableEC ec = HASH_TABLE_EC_Ok;
    size_t max_size = ht->max_size;
    for (size_t i=0; i<max_size; ++i) {
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

    for (size_t i=0; i<max_size; ++i) {
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
    HashTable* ht = NULL;
    const DataType* coord_type = datatype_create(1, cmp_coord, hash_coord, sizeof(Coord));
    const DataType* int_type = datatype_create(2, NULL, NULL, sizeof(int));
    EXPECT_TRUE(coord_type != NULL);
    EXPECT_TRUE(int_type != NULL);

    ht = hash_table_init(coord_type, int_type);
    EXPECT_TRUE(ht != NULL);

    HashTableEC ec = HASH_TABLE_EC_Undefined;
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) { 
        Coord key = {.x = (int)i, .y = (int)i};
        int val = (int)i;
        DataEnvelope* key_temp = envelope(coord_type, &key);
        DataEnvelope* val_temp = envelope(int_type, &val);
        ec = hash_table_insert(ht, key_temp, val_temp);
        EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);
    }
    Coord array[TEST_ARRAY_SIZE] = {};
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) { 
        Coord key = {.x = (int)i, .y = (int)i};
        array[i] = key;
    }
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) {
        printf("Key: X-%d Y-%d", array[i].x, array[i].y);
        DataEnvelope* key_temp = envelope(coord_type, &array[i]);
        DataEnvelope* out_val = NULL;
        ec = hash_table_find(ht, key_temp, &out_val);
        EXPECT_EQ_NUM(ec, HASH_TABLE_EC_Ok);
        EXPECT_EQ_NUM(*((int*)out_val->data), i);
    }

    TEAR_DOWN(
            s_envelope_free_all();
            datatype_free(coord_type);
            datatype_free(int_type);
            hash_table_free(ht);
            )
}
