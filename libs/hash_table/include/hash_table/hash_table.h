#ifndef CL_HASH_TABLE_HASH_TABLE_H
#define CL_HASH_TABLE_HASH_TABLE_H

#include <stddef.h>

#include "datatype/datatype.h"
#include "hash_table/errors.h"

typedef enum {
    KEY_STATE_Free,
    KEY_STATE_Busy,
    KEY_STATE_Deleted,
} KeyState;

typedef struct {
    DataEnvelope* key;
    KeyState state;
    DataEnvelope* val;
} KeySpace;

typedef struct {
    const DataType* key_type;
    const DataType* value_type;
    KeySpace* ks;
    size_t max_size;
    size_t current_size;
    size_t q;
} HashTable;

HashTable* hash_table_init(const DataType* key_type,
                           const DataType* value_type);
void hash_table_free(HashTable* ht);
HashTableEC hash_table_insert(HashTable* ht, DataEnvelope* key,
                              DataEnvelope* val);
HashTableEC hash_table_delete(HashTable* ht, DataEnvelope* key);
HashTableEC hash_table_find(HashTable* ht, DataEnvelope* key,
                            DataEnvelope** val);

#endif
