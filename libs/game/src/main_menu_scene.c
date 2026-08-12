#include <signal.h>
#include <sys/signalfd.h>
#include "game/main_menu_scene.h"
#include "zxcurses/menu.h"
#include "zxcurses/screen.h"
#include "zxcurses/event_listener.h"
#include "zxcurses/input_widget.h"
#include "zxcurses/panel.h"

typedef struct {
    Menu m;
    InputWidget* input;
    bool need_redraw;
    bool writing_mode;
    char* last_error;
} app_context;

bool on_stdin(int fd, void* cont) {
    (void)fd;
    app_context* ctx = (app_context*)cont;
    for (;;) {
        char letter = 0;
        PressedKey key = read_key(&letter);
        if (key == KEY_NOTHING) break;
        if (key == KEY_EOF) return false;
        switch (key) {
            case KEY_UP:
                input_widget_move_selection(ctx->input, 1);
                break;
            case KEY_DOWN:
                input_widget_move_selection(ctx->input, -1);
                break;
            case KEY_ENTER:
                ctx->writing_mode = !ctx->writing_mode;
                if (ctx->writing_mode) {
                    input_widget_highlight_on(ctx->input, COLOR_BLACK, COLOR_WHITE);
                } else {
                    input_widget_highlight_off(ctx->input);
                }
                break;
            case KEY_BACKSPACE:
                if (ctx->writing_mode) {
                    input_widget_remove_symbol(ctx->input);
                }
                break;
            case KEY_LETTER:
                if (ctx->writing_mode) {
                    input_widget_insert_symbol(ctx->input,letter);
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
    panel_draw_box(ctx->m.panel, COLOR_DEFAULT, COLOR_DEFAULT);
    menu_render(&ctx->m);
    input_widget_render(ctx->input);
    if (ctx->writing_mode) {
        panel_draw_text(ctx->input->panel, 0, 0, "Writing mode", COLOR_YELLOW, COLOR_DEFAULT);
    } else {
        panel_draw_text(ctx->input->panel, 0, 0, "Normal mode", COLOR_GREEN, COLOR_DEFAULT);
    }
    print_screen_buffer();
    ctx->need_redraw = false;
    return true;
}

void start_menu_scene() {
    init_view();

    const char* menu_items[] = {"Singleplayer", "Multiplayer", "Settings", "Quit"};
    termsize size = get_terminal_size();
    Panel panel = {.x=0,.y=0,.height=size.height,.width=size.width};
    InputWidget* widget = input_widget_init(panel);
    input_widget_add_entry(widget, "Single ship count: ", 10, "4");
    input_widget_add_entry(widget, "Double ship count: ", 10, "3");
    input_widget_add_entry(widget, "Triple ship count: ", 10, "2");
    input_widget_add_entry(widget, "Quadriple ship count: ", 10, "1");
    app_context ctx = {        
        .m = {.count=4, .items=menu_items, .selected=0},
        .input = widget,
        .need_redraw = true,
        .writing_mode = false,
    };
    init_event_listener();
    set_on_stdin(&on_stdin);
    set_on_signal(&on_signal);
    set_on_tick(&on_tick);

    create_screen_buffer(size.width, size.height);

    main_loop(&ctx);

    input_widget_free(widget);

    return 0;
}

