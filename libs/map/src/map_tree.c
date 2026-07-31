#include "map/map.h"
#include "tree/tree.h"
#include <stdlib.h>

static MapEC tree_error_to_map_error(TreeEC ec);

struct Map {
    Tree* tree;
};

Map* map_init(size_t size) {
    (void)size;
    Map* map = malloc(sizeof(Map));
    if (map == NULL) return NULL;

    const DataType* key_type = datatype_create(1, &cmp_coord, &hash_coord, sizeof(Coord));
    if (key_type == NULL) {free(map); return NULL;}

    const DataType* value_type = datatype_create(2, &cmp_int, NULL, sizeof(int));
    if (value_type == NULL) {free(map); datatype_free(key_type); return NULL;}

    map->tree = tree_init(key_type, value_type);
    if (map->tree == NULL) {free(map); datatype_free(key_type); datatype_free(value_type); return NULL;}

    return map;
}

void map_free(Map* m) {
    if (m == NULL) return;
    datatype_free(m->tree->key_type);
    datatype_free(m->tree->value_type);
    tree_free(m->tree);
    free(m);
}

MapEC map_insert(Map* m, Coord key, int value) {
    if (m == NULL) return MAP_ERR_IsNull;

    Tree* t = m->tree;
    
    DataEnvelope* key_envelope = envelope_create(t->key_type, &key);
    key_envelope->full_copy = false;
    DataEnvelope* value_envelope = envelope_create(t->value_type, &value);
    value_envelope->full_copy = false;

    TreeEC ec = tree_insert(t, key_envelope, value_envelope);

    return tree_error_to_map_error(ec);
}

MapEC map_delete(Map* m, Coord key) {
    if (m == NULL) return MAP_ERR_IsNull;
    Tree* t = m->tree;
    
    DataEnvelope* key_envelope = envelope_create(t->key_type, &key);

    TreeEC ec = tree_delete(t, key_envelope);

    envelope_free(key_envelope);

    return tree_error_to_map_error(ec);
}

MapEC map_find(Map* m, Coord key, int** out_value) {
    Tree* t = m->tree;
    
    DataEnvelope* key_envelope = envelope_create(t->key_type, &key);
    key_envelope->full_copy = false;
    DataEnvelope* res = NULL;

    TreeEC ec = tree_find(t, key_envelope, &res);
    if (ec == TREE_EC_Ok) {
        *out_value = (int*)res->data;
    }

    envelope_free(key_envelope);

    return tree_error_to_map_error(ec);
}

static MapEC tree_error_to_map_error(TreeEC ec) {
    switch (ec) {
        case TREE_EC_AllocationError:
            return MAP_ERR_AllocationError;
        case TREE_EC_Ok:
            return MAP_ERR_Ok;
        case TREE_EC_KeyDoesntExists:
            return MAP_ERR_KeyDoesntExists;
        case TREE_EC_IsNull:
            return MAP_ERR_IsNull;
        case TREE_EC_IncorrectType:
        case TREE_EC_CompareFail:
        case TREE_EC_EntryOverflow:
        case TREE_EC_UnexpectedError:
            return MAP_ERR_InternalError;
        case TREE_EC_UndefinedError:
            return MAP_ERR_Undefined;
        case TREE_EC_KeyExists:
            return MAP_ERR_KeyAlreadyExists;
    }
    return MAP_ERR_Undefined;
}
