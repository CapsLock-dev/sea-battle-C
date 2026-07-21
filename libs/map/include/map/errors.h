#ifndef CL_MAP_ERRORS_H
#define CL_MAP_ERRORS_H

typedef enum {
    MAP_ERR_Undefined,
    MAP_ERR_AllocationError,
    MAP_ERR_IsNull,
    MAP_ERR_KeyDoesntExists,
    MAP_ERR_InternalError,
    MAP_ERR_KeyAlreadyExists,
    MAP_ERR_IsFull,
    MAP_ERR_Ok,
} MapEC;

#endif
