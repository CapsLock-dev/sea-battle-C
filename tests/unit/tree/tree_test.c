#include "tree/tree.h"

#include <stdio.h>
#include <stdlib.h>

#include "cltest/cltest.h"
#include "test-utils/array_tools.h"
#include "test-utils/envelope_destructor.h"

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

#define envelope(type, data) s_envelope_init(envelope_create(type, data))

TEST_F(TreeInit) {
    Tree* tree = NULL;
    const DataType* coord_type =
        datatype_create(1, cmp_coord, NULL, sizeof(Coord));
    const DataType* int_type = datatype_create(2, NULL, NULL, sizeof(int));
    EXPECT_TRUE(coord_type != NULL);
    EXPECT_TRUE(int_type != NULL);

    tree = tree_init(coord_type, int_type);
    EXPECT_TRUE(tree != NULL);

    TEAR_DOWN(datatype_free(coord_type); datatype_free(int_type);
              tree_free(tree);)
}

#define get_coord(x) ((Coord*)x->data)
#define TEST_ARRAY_SIZE 1000

TEST_F(TreeInsert) {
    Coord a = {.x = 1, .y = 1};
    Coord b = {.x = 2, .y = 1};
    Coord c = {.x = 3, .y = 1};
    Coord d = {.x = 4, .y = 1};
    Coord e = {.x = 5, .y = 1};
    Coord f = {.x = 6, .y = 1};
    Coord g = {.x = 7, .y = 1};
    int a1 = 1;
    Tree* tree = NULL;
    const DataType* coord_type =
        datatype_create(1, cmp_coord, NULL, sizeof(Coord));
    const DataType* int_type = datatype_create(2, NULL, NULL, sizeof(int));
    EXPECT_TRUE(coord_type != NULL);
    EXPECT_TRUE(int_type != NULL);

    tree = tree_init(coord_type, int_type);
    EXPECT_TRUE(tree != NULL);

    DataEnvelope* key_a = envelope(coord_type, &a);
    DataEnvelope* key_b = envelope(coord_type, &b);
    DataEnvelope* key_c = envelope(coord_type, &c);
    DataEnvelope* key_d = envelope(coord_type, &d);
    DataEnvelope* key_e = envelope(coord_type, &e);
    DataEnvelope* key_f = envelope(coord_type, &f);
    DataEnvelope* key_g = envelope(coord_type, &g);
    DataEnvelope* val_a = envelope(int_type, &a1);

    TreeEC ec = tree_insert(tree, key_a, val_a);
    EXPECT_EQ_NUM(ec, TREE_EC_Ok);
    ec = tree_insert(tree, key_a, val_a);
    EXPECT_EQ_NUM(ec, TREE_EC_KeyExists);
    ec = tree_insert(tree, key_b, val_a);
    EXPECT_EQ_NUM(ec, TREE_EC_Ok);
    ec = tree_insert(tree, NULL, NULL);
    EXPECT_EQ_NUM(ec, TREE_EC_IsNull);

    ec = tree_insert(tree, key_c, val_a);
    EXPECT_EQ_NUM(ec, TREE_EC_Ok);
    EXPECT_EQ_NUM(get_coord(key_a)->x,
                  get_coord(tree->head->entries[0].key)->x);
    EXPECT_EQ_NUM(get_coord(key_b)->x,
                  get_coord(tree->head->entries[1].key)->x);
    EXPECT_EQ_NUM(get_coord(key_c)->x,
                  get_coord(tree->head->entries[2].key)->x);
    // Tree:
    // 1,2,3

    ec = tree_insert(tree, key_d, val_a);
    EXPECT_EQ_NUM(ec, TREE_EC_Ok);
    // Tree:
    //   2
    // 1  3,4
    EXPECT_EQ_NUM(get_coord(key_b)->x,
                  get_coord(tree->head->entries[0].key)->x);
    EXPECT_EQ_NUM(get_coord(key_a)->x,
                  get_coord(tree->head->children[0]->entries[0].key)->x);
    EXPECT_EQ_NUM(get_coord(key_c)->x,
                  get_coord(tree->head->children[1]->entries[0].key)->x);
    EXPECT_EQ_NUM(get_coord(key_d)->x,
                  get_coord(tree->head->children[1]->entries[1].key)->x);

    ec = tree_insert(tree, key_e, val_a);
    EXPECT_EQ_NUM(ec, TREE_EC_Ok);
    // Tree:
    //   2
    // 1  3,4,5
    EXPECT_EQ_NUM(get_coord(key_e)->x,
                  get_coord(tree->head->children[1]->entries[2].key)->x);

    ec = tree_insert(tree, key_f, val_a);
    EXPECT_EQ_NUM(ec, TREE_EC_Ok);
    // Tree:
    //   2,4
    // 1  3  5,6
    EXPECT_EQ_NUM(get_coord(key_b)->x,
                  get_coord(tree->head->entries[0].key)->x);
    EXPECT_EQ_NUM(get_coord(key_d)->x,
                  get_coord(tree->head->entries[1].key)->x);
    EXPECT_EQ_NUM(get_coord(key_a)->x,
                  get_coord(tree->head->children[0]->entries[0].key)->x);
    EXPECT_EQ_NUM(get_coord(key_c)->x,
                  get_coord(tree->head->children[1]->entries[0].key)->x);
    EXPECT_EQ_NUM(get_coord(key_e)->x,
                  get_coord(tree->head->children[2]->entries[0].key)->x);
    EXPECT_EQ_NUM(get_coord(key_f)->x,
                  get_coord(tree->head->children[2]->entries[1].key)->x);

    Coord array[TEST_ARRAY_SIZE] = {};
    for (size_t i = 0; i < TEST_ARRAY_SIZE; ++i) {
        Coord key = {.x = (int)i + f.x, .y = (int)i};
        array[i] = key;
    }
    array_shuffle(array, TEST_ARRAY_SIZE, sizeof(Coord));
    for (size_t i = 10; i < TEST_ARRAY_SIZE; ++i) {
        printf("Key: X-%d Y-%d", array[i].x, array[i].y);
        DataEnvelope* key_temp = envelope(coord_type, &array[i]);
        ec = tree_insert(tree, key_temp, val_a);
        EXPECT_EQ_NUM(ec, TREE_EC_Ok);
    }

    TEAR_DOWN(s_envelope_free_all(); datatype_free(coord_type);
              datatype_free(int_type); tree_free(tree);)
}

TEST_F(TreeDelete) {
    Tree* tree = NULL;
    const DataType* coord_type =
        datatype_create(1, cmp_coord, NULL, sizeof(Coord));
    const DataType* int_type = datatype_create(2, NULL, NULL, sizeof(int));
    EXPECT_TRUE(coord_type != NULL);
    EXPECT_TRUE(int_type != NULL);

    tree = tree_init(coord_type, int_type);
    EXPECT_TRUE(tree != NULL);

    TreeEC ec = TREE_EC_UndefinedError;
    for (int j = 0; j < 10; ++j) {
        for (size_t i = 0; i < TEST_ARRAY_SIZE; ++i) {
            Coord key = {.x = (int)i, .y = (int)i};
            int val = (int)i;
            DataEnvelope* key_temp = envelope(coord_type, &key);
            DataEnvelope* val_temp = envelope(int_type, &val);
            ec = tree_insert(tree, key_temp, val_temp);
            EXPECT_EQ_NUM(ec, TREE_EC_Ok);
        }
        Coord array[TEST_ARRAY_SIZE] = {};
        for (size_t i = 0; i < TEST_ARRAY_SIZE; ++i) {
            Coord key = {.x = (int)i, .y = (int)i};
            array[i] = key;
        }
        array_shuffle(array, TEST_ARRAY_SIZE, sizeof(Coord));
        for (size_t i = 0; i < TEST_ARRAY_SIZE; ++i) {
            printf("Key: X-%d Y-%d", array[i].x, array[i].y);
            DataEnvelope* key_temp = envelope(coord_type, &array[i]);
            ec = tree_delete(tree, key_temp);
            EXPECT_EQ_NUM(ec, TREE_EC_Ok);
        }
    }

    TEAR_DOWN(s_envelope_free_all(); datatype_free(coord_type);
              datatype_free(int_type); tree_free(tree);)
}

TEST_F(TreeFind) {
    Tree* tree = NULL;
    const DataType* coord_type =
        datatype_create(1, cmp_coord, NULL, sizeof(Coord));
    const DataType* int_type = datatype_create(2, NULL, NULL, sizeof(int));
    EXPECT_TRUE(coord_type != NULL);
    EXPECT_TRUE(int_type != NULL);

    tree = tree_init(coord_type, int_type);
    EXPECT_TRUE(tree != NULL);

    TreeEC ec = TREE_EC_UndefinedError;
    for (size_t i = 0; i < TEST_ARRAY_SIZE; ++i) {
        Coord key = {.x = (int)i, .y = (int)i};
        int val = (int)i;
        DataEnvelope* key_temp = envelope(coord_type, &key);
        DataEnvelope* val_temp = envelope(int_type, &val);
        ec = tree_insert(tree, key_temp, val_temp);
        EXPECT_EQ_NUM(ec, TREE_EC_Ok);
    }
    Coord array[TEST_ARRAY_SIZE] = {};
    for (size_t i = 0; i < TEST_ARRAY_SIZE; ++i) {
        Coord key = {.x = (int)i, .y = (int)i};
        array[i] = key;
    }
    for (size_t i = 0; i < TEST_ARRAY_SIZE; ++i) {
        printf("Key: X-%d Y-%d", array[i].x, array[i].y);
        DataEnvelope* key_temp = envelope(coord_type, &array[i]);
        DataEnvelope* out_val = NULL;
        ec = tree_find(tree, key_temp, &out_val);
        EXPECT_EQ_NUM(ec, TREE_EC_Ok);
        EXPECT_EQ_NUM(*((int*)out_val->data), i);
    }

    TEAR_DOWN(s_envelope_free_all(); datatype_free(coord_type);
              datatype_free(int_type); tree_free(tree);)
}
