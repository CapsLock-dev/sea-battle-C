#include "core/app.h"
#include "zxcurses/terminal.h"
#include "zxcurses/screen.h"
#include "zxcurses/event_listener.h"
#include <signal.h>
#include <stdio.h>
#include <sys/signalfd.h>

bool on_stdin(int fd) {
    unsigned char buf[64];
    for (;;) {
        ssize_t r = read(fd, buf, sizeof(buf));
        if (r < 0) {
            return false;
        }
        if (r == 0) return false; // EOF
        for (ssize_t k = 0; k < r; k++) {
            unsigned char c = buf[k];
            if (c == 'q' || c == 'Q' || c == 27 /*ESC*/) {
                return false;
            }
        }
    }
    return true;
}

bool on_signal(int fd) {
    struct signalfd_siginfo si;
    for (;;) {
        ssize_t r = read(fd, &si, sizeof(si));
        if (r < 0) {
            break;
        }
        if (r != sizeof(si)) break;
        if (si.ssi_signo == SIGINT) {
            return false;
        } else if (si.ssi_signo == SIGWINCH) {
            termsize size = get_terminal_size();
            resize_screen_buffer(size.width, size.height);
            write(STDOUT_FILENO, "\033[2J\033[H", 7);
        }
    }
    return true;
}

bool on_timer(int fd) {
    static int timer = 0;
    uint64_t expirations = 0;
    if (read(fd, &expirations, sizeof(expirations)) != sizeof(expirations)) {}
    char buffer[20] = {};
    sprintf(buffer, "Timer: %ds", timer++);
    draw_text(3,2,buffer);
    print_screen_buffer();
    return true;
}

int run(int argc, char** argv) {
    (void)argc; (void)argv;
    init_view();

    init_event_listener();
    set_on_stdin(&on_stdin);
    set_on_signal(&on_signal);
    set_on_timer(&on_timer);

    termsize size = get_terminal_size();
    create_screen_buffer(size.width, size.height);

    main_loop();

    return 0;
}
