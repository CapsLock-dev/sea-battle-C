#include "cltest/cltest.h"
#include "map/map.h"
#include "test-utils/array_tools.h"
#include <stdlib.h>

#define envelope(type, data) s_envelope_init(envelope_create(type, data))

TEST_F(MapInit) {
    Map* map = map_init(100);
    EXPECT_TRUE(map != NULL);

    TEAR_DOWN(
        map_free(map);
    )
}
#define TEST_ARRAY_SIZE 500
TEST_F(MapInsert) {
    Map* map = map_init(1000);
    EXPECT_TRUE(map != NULL);

    MapEC ec = MAP_ERR_Undefined;
    Coord array[TEST_ARRAY_SIZE] = {};
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) { 
        Coord key = {.x = (int)i, .y = (int)i};
        array[i] = key;
    }
    array_shuffle(array, TEST_ARRAY_SIZE, sizeof(Coord));
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) {
        printf("Key: X-%d Y-%d", array[i].x, array[i].y);
        ec = map_insert(map, array[i], (int)i);
        EXPECT_EQ_NUM(ec, MAP_ERR_Ok);
    }

    TEAR_DOWN(
        map_free(map);
    )
}

TEST_F(MapDelete) {
    Map* map = map_init(1000);
    EXPECT_TRUE(map != NULL);

    MapEC ec = MAP_ERR_Undefined;
    Coord array[TEST_ARRAY_SIZE] = {};
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) { 
        Coord key = {.x = (int)i, .y = (int)i};
        array[i] = key;
    }
    array_shuffle(array, TEST_ARRAY_SIZE, sizeof(Coord));
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) {
        printf("Key: X-%d Y-%d", array[i].x, array[i].y);
        ec = map_insert(map, array[i], (int)i);
        EXPECT_EQ_NUM(ec, MAP_ERR_Ok);
    }
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) {
        printf("Key: X-%d Y-%d", array[i].x, array[i].y);
        ec = map_delete(map, array[i]);
        EXPECT_EQ_NUM(ec, MAP_ERR_Ok);
    }

    TEAR_DOWN(
        map_free(map);
    )
}

TEST_F(MapFind) {
    Map* map = map_init(1000);
    EXPECT_TRUE(map != NULL);

    MapEC ec = MAP_ERR_Undefined;
    Coord array[TEST_ARRAY_SIZE] = {};
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) { 
        Coord key = {.x = (int)i, .y = (int)i};
        array[i] = key;
    }
    array_shuffle(array, TEST_ARRAY_SIZE, sizeof(Coord));
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) {
        printf("Key: X-%d Y-%d", array[i].x, array[i].y);
        ec = map_insert(map, array[i], (int)i);
        EXPECT_EQ_NUM(ec, MAP_ERR_Ok);
    }
    for (size_t i=0; i<TEST_ARRAY_SIZE; ++i) {
        printf("Key: X-%d Y-%d", array[i].x, array[i].y);
        int* res = NULL;
        ec = map_find(map, array[i], &res);
        EXPECT_EQ_NUM(ec, MAP_ERR_Ok);
        EXPECT_EQ_NUM(*res, (int)i);
    }

    TEAR_DOWN(
        map_free(map);
    )
}
