#include "core/app.h"
#include "zxcurses/menu.h"
#include "zxcurses/terminal.h"
#include "zxcurses/screen.h"
#include "zxcurses/event_listener.h"
#include "zxcurses/panel.h"
#include <signal.h>
#include <sys/signalfd.h>

typedef struct {
    Menu m;
    bool need_redraw;
} app_context;

bool on_stdin(int fd, void* cont) {
    app_context* ctx = (app_context*)cont;
    for (;;) {
        char letter = 0;
        PressedKey key = read_key(&letter);
        if (key == KEY_NOTHING) break;
        if (key == KEY_EOF) return false;
        switch (key) {
            case KEY_UP:
                menu_move_selection(&ctx->m, 1);
                break;
            case KEY_DOWN:
                menu_move_selection(&ctx->m, -1);
                break;
            default:
                break;
        }
        menu_render(&ctx->m);
        print_screen_buffer();
        ctx->need_redraw = true;
        break;
    }
    return true;
}

bool on_signal(int fd, void* context) {
    app_context* ctx = (app_context*)context;
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
            ctx->need_redraw = true;
        }
    }
    return true;
}

bool on_tick(void* context) {
    app_context* ctx = (app_context*)context;
    termsize size = get_terminal_size();
    Panel panel = {.x=0,.y=0,.height=size.height,.width=size.width};
    ctx->m.panel = panel;
    panel_draw_box(panel, COLOR_DEFAULT, COLOR_DEFAULT);
    menu_render(&ctx->m);
    print_screen_buffer();
    ctx->need_redraw = false;
    return true;
}

int run(int argc, char** argv) {
    (void)argc; (void)argv;
    init_view();


    const char* menu_items[] = {"Singleplayer", "Multiplayer", "Settings", "Quit"};
    app_context ctx = {        
        .m = {.count=4, .items=menu_items, .selected=0},
        .need_redraw = true,
    };
    init_event_listener();
    set_on_stdin(&on_stdin);
    set_on_signal(&on_signal);
    set_on_tick(&on_tick);

    termsize size = get_terminal_size();
    create_screen_buffer(size.width, size.height);

    main_loop(&ctx);

    return 0;
}
