#include "zxcurses/terminal.h"
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

static void sigint_handler(int signum) {
    end_view();
    exit(signum);
}
static struct termios old_settings;
static struct termios current_settings;

void init_view() {
    signal(SIGINT, sigint_handler);
    tcgetattr(STDIN_FILENO, &old_settings);
    current_settings = old_settings;
    current_settings.c_lflag &= ~(unsigned int)(ICANON | ECHO); 
    current_settings.c_cc[VTIME] = 0;
    current_settings.c_cc[VMIN] = 1;

    tcsetattr(STDIN_FILENO, TCSANOW, &current_settings);
    printf("\033[?25l");
    printf("\033[1;1H\033[2J");
    fflush(stdout);
}

PressedKey read_key(char* letter) {
	char c = 0;
    read(STDIN_FILENO, &c, 1);
    if (c == 0x04) return KEY_EOF;
    if (c == '\n') return KEY_ENTER;
    if (c == 0x7f) return KEY_BACKSPACE;
    if (c == 0x57) return KEY_SPACE;
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
    printf("\033[?25h");
    fflush(stdout);
    tcsetattr(0, TCSANOW, &old_settings); 
}
