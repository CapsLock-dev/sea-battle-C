#include "hash_table/hash_table.h"

#include <stdio.h>
#include <stdlib.h>

static size_t compute_step(size_t max_size) {
    size_t primes[] = {3, 5, 7, 11, 13, 17, 19, 23};
    for (size_t i = 0; i < sizeof(primes) / sizeof(primes[0]); ++i) {
        if (max_size % primes[i] != 0) {
            return primes[i];
        }
    }
    return 1;
}

#define INIT_HASHTABLE_SIZE 64

HashTable* hash_table_init(const DataType* key_type,
                           const DataType* value_type) {
    HashTable* ht = malloc(sizeof(HashTable));
    if (ht == NULL) return NULL;
    ht->ks = calloc(INIT_HASHTABLE_SIZE, sizeof(KeySpace));
    if (ht->ks == NULL) {
        free(ht);
        return NULL;
    }
    ht->max_size = INIT_HASHTABLE_SIZE;
    ht->current_size = 0;
    ht->key_type = key_type;
    ht->value_type = value_type;
    ht->q = compute_step(INIT_HASHTABLE_SIZE);
    return ht;
}

static void keyspace_free(KeySpace* ks) {
    if (ks == NULL) return;
    if (ks->state == KEY_STATE_Busy) {
        envelope_free(ks->val);
        envelope_free(ks->key);
    }
}

void hash_table_free(HashTable* ht) {
    if (ht == NULL) return;
    for (size_t i = 0; i < ht->max_size; ++i) {
        keyspace_free(&ht->ks[i]);
    }
    free(ht->ks);
    free(ht);
}

HashTableEC hash_table_insert(HashTable* ht, DataEnvelope* key,
                              DataEnvelope* val) {
    if (ht == NULL || key == NULL || val == NULL) return HASH_TABLE_EC_IsNull;
    if (key->type != ht->key_type || val->type != ht->value_type)
        return HASH_TABLE_EC_IncorrectType;

    if (ht->current_size * 100 >= ht->max_size * 70) {
        size_t new_size = ht->max_size * 2;
        KeySpace* new_ks = calloc(new_size, sizeof(KeySpace));
        if (new_ks == NULL) return HASH_TABLE_EC_AllocationError;
        size_t q = compute_step(new_size);
        for (size_t i = 0; i < ht->max_size; ++i) {
            if (ht->ks[i].state == KEY_STATE_Busy) {
                KeySpace* target = &ht->ks[i];
                size_t hash = ht->key_type->hash(target->key);
                for (size_t j = 0; j < new_size; ++j) {
                    size_t index = (hash + j * q) % new_size;
                    KeySpace* new_place = &new_ks[index];
                    if (new_place->state != KEY_STATE_Busy) {
                        new_place->key = target->key;
                        new_place->state = KEY_STATE_Busy;
                        new_place->val = target->val;
                        break;
                    }
                }
            }
        }
        free(ht->ks);
        ht->ks = new_ks;
        ht->q = q;
        ht->max_size = new_size;
    }

    size_t hash = ht->key_type->hash(key);

    size_t first_deleted = ht->max_size + 1;
    for (size_t i = 0; i < ht->max_size; ++i) {
        size_t index = (hash + i * ht->q) % ht->max_size;

        KeySpace* ks = &ht->ks[index];
        if (ks->state == KEY_STATE_Busy) {
            CompareResult cmp_res = ht->key_type->cmp(ks->key, key);
            if (cmp_res == CMP_EQUAL) {
                return HASH_TABLE_EC_KeyExists;
            } else if (cmp_res == CMP_WRONG) {
                return HASH_TABLE_EC_CompareFail;
            }
        } else {
            if (ks->state == KEY_STATE_Deleted &&
                first_deleted == ht->max_size + 1) {
                first_deleted = index;
                continue;
            }
            if (first_deleted != ht->max_size + 1) {
                ks = &ht->ks[first_deleted];
            }
            ks->key = envelope_copy(key);
            if (ks->key == NULL) return HASH_TABLE_EC_AllocationError;
            ks->val = envelope_copy(val);
            if (ks->val == NULL) {
                envelope_free(ks->key);
                return HASH_TABLE_EC_AllocationError;
            }
            ks->state = KEY_STATE_Busy;
            ++ht->current_size;
            return HASH_TABLE_EC_Ok;
        }
    }
    if (first_deleted != ht->max_size + 1) {
        KeySpace* ks = &ht->ks[first_deleted];
        ks->key = envelope_copy(key);
        if (ks->key == NULL) return HASH_TABLE_EC_AllocationError;
        ks->val = envelope_copy(val);
        if (ks->val == NULL) {
            envelope_free(ks->key);
            return HASH_TABLE_EC_AllocationError;
        }
        ks->state = KEY_STATE_Busy;
        ++ht->current_size;
        return HASH_TABLE_EC_Ok;
    }
    return HASH_TABLE_EC_UnexpectedError;
}

HashTableEC hash_table_delete(HashTable* ht, DataEnvelope* key) {
    if (ht == NULL || key == NULL) return HASH_TABLE_EC_IsNull;
    if (key->type != ht->key_type) return HASH_TABLE_EC_IncorrectType;

    size_t hash = ht->key_type->hash(key);

    for (size_t i = 0; i < ht->max_size; ++i) {
        size_t index = (hash + i * ht->q) % ht->max_size;

        KeySpace* ks = &ht->ks[index];

        if (ks->state == KEY_STATE_Busy) {
            CompareResult cmp_res = ht->key_type->cmp(ks->key, key);
            if (cmp_res == CMP_EQUAL) {
                envelope_free(ks->key);
                envelope_free(ks->val);
                ks->val = NULL;
                ks->key = NULL;
                ks->state = KEY_STATE_Deleted;
                --ht->current_size;
                return HASH_TABLE_EC_Ok;
            } else if (cmp_res == CMP_WRONG) {
                return HASH_TABLE_EC_CompareFail;
            }
        }
    }
    return HASH_TABLE_EC_KeyDoesntExists;
}

HashTableEC hash_table_find(HashTable* ht, DataEnvelope* key,
                            DataEnvelope** val) {
    if (ht == NULL || key == NULL) return HASH_TABLE_EC_IsNull;
    if (key->type != ht->key_type) return HASH_TABLE_EC_IncorrectType;

    size_t hash = ht->key_type->hash(key);

    for (size_t i = 0; i < ht->max_size; ++i) {
        size_t index = (hash + i * ht->q) % ht->max_size;

        KeySpace* ks = &ht->ks[index];
        if (ks->state == KEY_STATE_Free) return HASH_TABLE_EC_KeyDoesntExists;

        if (ks->state == KEY_STATE_Busy) {
            CompareResult cmp_res = ht->key_type->cmp(ks->key, key);
            if (cmp_res == CMP_EQUAL) {
                *val = ks->val;
                return HASH_TABLE_EC_Ok;
            } else if (cmp_res == CMP_WRONG) {
                return HASH_TABLE_EC_CompareFail;
            }
        }
    }
    return HASH_TABLE_EC_KeyDoesntExists;
}
