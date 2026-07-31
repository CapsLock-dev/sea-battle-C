#include "map/map.h"
#include "hash_table/hash_table.h"
#include <stdlib.h>
#include <string.h>

static MapEC hashtable_error_to_map_error(HashTableEC ec);
static CompareResult cmp_coord(const DataEnvelope* a, const DataEnvelope* b);
static CompareResult cmp_int(const DataEnvelope* a, const DataEnvelope* b);
static size_t hash_coord(const DataEnvelope* a);

struct Map {
    HashTable* table;
};

Map* map_init(size_t size) {
    Map* map = malloc(sizeof(Map));
    if (map == NULL) return NULL;

    const DataType* key_type = datatype_create(1, &cmp_coord, &hash_coord, sizeof(Coord));
    if (key_type == NULL) {free(map); return NULL;}

    const DataType* value_type = datatype_create(2, &cmp_int, NULL, sizeof(int));
    if (value_type == NULL) {free(map); datatype_free(key_type); return NULL;}

    map->table = hash_table_init(size, key_type, value_type);
    if (map->table == NULL) {free(map); datatype_free(key_type); datatype_free(value_type); return NULL;}

    return map;
}

void map_free(Map* m) {
    if (m == NULL) return;
    hash_table_free(m->table);
    free(m);
}

MapEC map_insert(Map* m, Coord key, int value) {
    if (m == NULL) return MAP_ERR_IsNull;

    HashTable* t = m->table;

    DataEnvelope* key_envelope = envelope_create(t->key_type, &key);
    key_envelope->full_copy = false;
    DataEnvelope* value_envelope = envelope_create(t->value_type, &value);
    value_envelope->full_copy = false;

    HashTableEC ec = hash_table_insert(m->table, key_envelope, value_envelope);

    return hashtable_error_to_map_error(ec);
}

MapEC map_delete(Map* m, Coord key) {
    if (m == NULL) return MAP_ERR_IsNull;

    HashTable* t = m->table;

    DataEnvelope* key_envelope = envelope_create(t->key_type, &key);
    key_envelope->full_copy = false;

    HashTableEC ec = hash_table_delete(m->table, key_envelope);

    return hashtable_error_to_map_error(ec);
}

MapEC map_find(Map* m, Coord key, int** out_value) {
    if (m == NULL) return MAP_ERR_IsNull;

    HashTable* t = m->table;

    DataEnvelope* key_envelope = envelope_create(t->key_type, &key);
    key_envelope->full_copy = false;
    DataEnvelope* value_envelope = NULL;

    HashTableEC ec = hash_table_find(m->table, key_envelope, &value_envelope);
    if (value_envelope != NULL) {
        *out_value = (int*)value_envelope->data;
    }

    return hashtable_error_to_map_error(ec);
}

static MapEC hashtable_error_to_map_error(HashTableEC ec) {
    switch (ec) {
        case HASH_TABLE_EC_AllocationError:
            return MAP_ERR_AllocationError;
        case HASH_TABLE_EC_Ok:
            return MAP_ERR_Ok;
        case HASH_TABLE_EC_KeyDoesntExists:
            return MAP_ERR_KeyDoesntExists;
        case HASH_TABLE_EC_IsNull:
            return MAP_ERR_IsNull;
        case HASH_TABLE_EC_IncorrectType:
        case HASH_TABLE_EC_CompareFail:
            return MAP_ERR_InternalError;
        case HASH_TABLE_EC_Full:
            return MAP_ERR_IsFull;
        case HASH_TABLE_EC_Undefined:
            return MAP_ERR_Undefined;
        case HASH_TABLE_EC_KeyExists:
            return MAP_ERR_KeyAlreadyExists;
    }
}

