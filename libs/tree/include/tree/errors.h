#ifndef CL_TREE_ERRORS_H
#define CL_TREE_ERRORS_H

typedef enum {
    TREE_EC_UndefinedError,
    TREE_EC_AllocationError,
    TREE_EC_IsNull,
    TREE_EC_IncorrectType,
    TREE_EC_CompareFail,
    TREE_EC_EntryOverflow,

    TREE_EC_KeyExists,
    TREE_EC_KeyDoesntExists,
    TREE_EC_UnexpectedError,

    TREE_EC_Ok,
} TreeEC;

#endif
