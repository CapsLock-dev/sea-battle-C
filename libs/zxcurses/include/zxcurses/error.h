#ifndef CL_ZXCURSES_ERROR_H
#define CL_ZXCURSES_ERROR_H

typedef enum {
    TUI_EC_UndefinedError,
    TUI_EC_AllocationError,
    TUI_EC_OutOfBounds,
    TUI_EC_TerminalTooSmall,
    TUI_EC_IsNull,
    TUI_EC_Ok,
} TUIError;

#endif
