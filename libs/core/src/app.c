#include "core/app.h"
#include "zxcurses/menu.h"
#include "zxcurses/terminal.h"
#include "zxcurses/screen.h"
#include "zxcurses/event_listener.h"
#include "zxcurses/panel.h"
#include <signal.h>
#include <sys/signalfd.h>

static const char* menu_items[] = {"Singleplayer", "Multiplayer", "Settings", "Quit"};
static Menu m = {.count=4, .items=menu_items, .selected=0};
static bool need_redraw = true;

bool on_stdin(int fd) {
    for (;;) {
        char letter = 0;
        PressedKey key = read_key(&letter);
        if (key == KEY_NOTHING) break;
        if (key == KEY_EOF) return false;
        switch (key) {
            case KEY_UP:
                menu_move_selection(&m, 1);
                break;
            case KEY_DOWN:
                menu_move_selection(&m, -1);
                break;
            default:
                break;
        }
        menu_render(&m);
        print_screen_buffer();
        need_redraw = true;
        break;
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
            need_redraw = true;
        }
    }
    return true;
}

bool on_tick(void) {
    termsize size = get_terminal_size();
    Panel panel = {.x=0,.y=0,.height=size.height,.width=size.width};
    m.panel = panel;
    panel_draw_box(panel, COLOR_DEFAULT, COLOR_DEFAULT);
    menu_render(&m);
    print_screen_buffer();
    need_redraw = false;
    return true;
}

int run(int argc, char** argv) {
    (void)argc; (void)argv;
    init_view();

    init_event_listener();
    set_on_stdin(&on_stdin);
    set_on_signal(&on_signal);
    set_on_tick(&on_tick);

    termsize size = get_terminal_size();
    create_screen_buffer(size.width, size.height);

    main_loop();

    return 0;
}
