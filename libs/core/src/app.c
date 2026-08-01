#include "core/app.h"
#include "zxcurses/terminal.h"
#include <stdio.h>
#include <sys/epoll.h>

void listener() {
    termsize size = get_terminal_size();
    printf("Console size: %d %d \n", size.width, size.height);
}

int run(int argc, char** argv) {
    init_view();
    bool running = true;
    while(running) {
        PressedKey key = read_key(NULL);
        if (key == KEY_EOF) running = false;
    }
    end_view();
    return 0;
}
