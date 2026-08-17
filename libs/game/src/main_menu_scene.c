#include "game/main_menu_scene.h"

#include <signal.h>
#include <sys/signalfd.h>

#include "zxcurses/event_listener.h"
#include "zxcurses/menu.h"
#include "zxcurses/panel.h"
#include "zxcurses/screen.h"

typedef struct {
    Menu m;
    bool need_redraw;
    MainMenuOption main_menu_option;
} app_context;

bool on_stdin_mainmenu(int fd, void* cont) {
    (void)fd;
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
            case KEY_ENTER:
                switch (ctx->m.selected) {
                    case MAIN_MENU_OPTION_SINGLEPLAYER:
                        ctx->main_menu_option = (MainMenuOption)ctx->m.selected;
                        return false;
                    case MAIN_MENU_OPTION_MULTIPLAYER:
                        ctx->main_menu_option = (MainMenuOption)ctx->m.selected;
                        return false;
                    case MAIN_MENU_OPTION_SETTINGS:
                        ctx->main_menu_option = (MainMenuOption)ctx->m.selected;
                        return false;
                    case MAIN_MENU_OPTION_QUIT:
                        ctx->main_menu_option = (MainMenuOption)ctx->m.selected;
                        return false;
                }
                break;
            default:
                break;
        }
        ctx->need_redraw = true;
        break;
    }
    return true;
}

bool on_signal_mainmenu(int fd, void* context) {
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
            ctx->m.panel = (Panel){.x = (size.width - 15) / 2,
                                   .y = (size.height - 6) / 2,
                                   .height = 6,
                                   .width = 15};
            ctx->need_redraw = true;
        }
    }
    return true;
}

bool on_tick_mainmenu(void* context) {
    app_context* ctx = (app_context*)context;
    menu_render(&ctx->m);
    if (ctx->need_redraw) print_screen_buffer();
    ctx->need_redraw = false;
    return true;
}

MainMenuOption start_main_menu_scene() {
    clear_screen_buffer();
    init_event_listener();
    const char* menu_items[] = {"Singleplayer", "Multiplayer", "Settings",
                                "Quit"};
    termsize size = get_terminal_size();
    Panel panel = {.x = (size.width - 15) / 2,
                   .y = (size.height - 6) / 2,
                   .height = 6,
                   .width = 15};
    app_context ctx = {
        .m = {.panel = panel, .count = 4, .items = menu_items, .selected = 0},
        .need_redraw = true,
        .main_menu_option = MAIN_MENU_OPTION_QUIT};
    set_on_stdin(&on_stdin_mainmenu);
    set_on_signal(&on_signal_mainmenu);
    set_on_tick(&on_tick_mainmenu);

    main_loop(&ctx);

    return ctx.main_menu_option;
}
