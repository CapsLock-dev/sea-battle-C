#ifndef CL_HASH_TABLE_ERRORS_H
#define CL_HASH_TABLE_ERRORS_H

typedef enum {
    HASH_TABLE_EC_Undefined,
    HASH_TABLE_EC_AllocationError,
    HASH_TABLE_EC_IsNull,
    HASH_TABLE_EC_IncorrectType,
    HASH_TABLE_EC_CompareFail,

    HASH_TABLE_EC_KeyExists,
    HASH_TABLE_EC_KeyDoesntExists,
    HASH_TABLE_EC_Full,

    HASH_TABLE_EC_Ok,
} HashTableEC;

#endif
