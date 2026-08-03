#ifndef CL_ZXCURSES_TERMINAL_H
#define CL_ZXCURSES_TERMINAL_H
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

typedef enum {
    KEY_NOTHING,

    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,

    KEY_ENTER,
    KEY_SPACE,
    KEY_BACKSPACE,

	KEY_LETTER,
    KEY_EOF,
} PressedKey;

typedef struct {
	unsigned short int width;
	unsigned short int height;
} termsize;

void init_view();
void end_view();
void clear_terminal();
void move_cursor(unsigned short int x, unsigned short int y);
PressedKey read_key(char* letter);
termsize get_terminal_size();

#endif
