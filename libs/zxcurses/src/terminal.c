#include "zxcurses/terminal.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static struct termios old_settings;
static struct termios current_settings;

void init_view() {
    tcgetattr(STDIN_FILENO, &old_settings);
    current_settings = old_settings;
    current_settings.c_lflag &= ~(unsigned int)(ICANON | ECHO); 
    current_settings.c_cc[VTIME] = 0;
    current_settings.c_cc[VMIN] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &current_settings);

    write(STDOUT_FILENO, "\033[?25l", 6);
    atexit(end_view);
}

PressedKey read_key(char* letter) {
	char c = 0;
    read(STDIN_FILENO, &c, 1);
    if (c == 0x04) return KEY_EOF;
    if (c == '\n') return KEY_ENTER;
    if (c == 0x7f) return KEY_BACKSPACE;
    if (c == ' ') return KEY_SPACE;
    if (c == 0x1B) {
        char code[2];
        ssize_t bytes = read(STDIN_FILENO, &code, 2);
        if (bytes == 2 && code[0] == '[') {
            switch (code[1]) {
                case 'A':
                    return KEY_UP;
                case 'B':
                    return KEY_DOWN;
                case 'C': 
                    return KEY_RIGHT; 
                case 'D': 
                    return KEY_LEFT;  
            }
        }
    }
	if(letter != NULL) *letter = c;
	return KEY_LETTER;

}

termsize get_terminal_size() {
    struct winsize ws = {};
    ioctl(0, TIOCGWINSZ, &ws);
    return (termsize){.width = ws.ws_col, .height = ws.ws_row};
}

void end_view() {
    tcsetattr(0, TCSANOW, &old_settings); 
    write(STDOUT_FILENO, "\033[?25h", 6);
}

void clear_terminal() {
    write(STDOUT_FILENO, "\033[2J", 4);
}

void move_cursor(TermSizeType x, TermSizeType y) {
    char buffer[100];
    sprintf(buffer, "\033[%d;%dH", y, x);
    write(STDOUT_FILENO, buffer, strlen(buffer));
}
