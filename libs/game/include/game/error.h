#ifndef CL_GAME_ERROR_H
#define CL_GAME_ERROR_H

typedef enum {
    GAME_EC_UndefinedError,
    GAME_EC_AllocationError,
    GAME_EC_InternalMapError,
    GAME_EC_CantPlaceHere,
    GAME_EC_PlacementFinished,
    GAME_EC_Ok,
} GameEC;

#endif
