#include <signal.h>
#include <sys/signalfd.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include "game/settings_scene.h"
#include "zxcurses/screen.h"
#include "zxcurses/event_listener.h"
#include "zxcurses/input_widget.h"
#include "zxcurses/panel.h"

typedef struct {
    InputWidget* input;
    bool need_redraw;
    bool writing_mode;
    char* last_error;
    GameSettings* settings;
} app_context;

static TermSizeType parse_input_value(char* str, char** error) {
    *error = NULL;
    if (str == NULL || *str == '\0') {
        *error = "Value is empty";
        return 0;
    }
    errno = 0;
    char *endptr;
    unsigned long val = strtoul(str, &endptr, 10);
    if (errno == ERANGE || val > USHRT_MAX) {
        *error = "Number too big or negative";
        return 0;
    }
    if (endptr == str) {
        *error = "Not a number";
        return 0;
    } 
    if (val == 0) {
        *error = "Can't be zero";
    }
    while (*endptr != '\0') {
        if (*endptr != ' ') {
            *error = "Not a number";
        }
        endptr++;
    }
    return (TermSizeType)val;
}

typedef enum {
    SETTINGS_OPTION_WIDTH,
    SETTINGS_OPTION_HEIGHT,
    SETTINGS_OPTION_SINGLE_SHIP,
    SETTINGS_OPTION_DUO_SHIP,
    SETTINGS_OPTION_TRIPLE_SHIP,
    SETTINGS_OPTION_QUADRIPLE_SHIP,
} SettingsOptions;

static void change_settings(GameSettings* settings, SettingsOptions option, TermSizeType value) {
    switch (option) {
        case SETTINGS_OPTION_WIDTH:
            settings->width = value;
            break;
        case SETTINGS_OPTION_HEIGHT:
            settings->height = value;
            break;
        case SETTINGS_OPTION_SINGLE_SHIP:
            settings->single_ship_count = value;
            break;
        case SETTINGS_OPTION_DUO_SHIP:
            settings->duo_ship_count = value;
            break;
        case SETTINGS_OPTION_TRIPLE_SHIP:
            settings->triple_ship_count = value;
            break;
        case SETTINGS_OPTION_QUADRIPLE_SHIP:
            settings->quadriple_ship_count = value;
            break;

    }
}

bool on_stdin_settings(int fd, void* cont) {
    (void)fd;
    app_context* ctx = (app_context*)cont;
    for (;;) {
        char letter = 0;
        PressedKey key = read_key(&letter);
        if (key == KEY_NOTHING) break;
        if (key == KEY_EOF) return false;
        switch (key) {
            case KEY_UP:
                if (!ctx->writing_mode) input_widget_move_selection(ctx->input, 1);
                break;
            case KEY_DOWN:
                if (!ctx->writing_mode) input_widget_move_selection(ctx->input, -1);
                break;
            case KEY_ENTER:
                if (ctx->writing_mode) {
                    // Exiting writing_mode
                    char* val = ctx->input->entries[ctx->input->selected].value;
                    char* err_msg = 0;
                    TermSizeType val_int = parse_input_value(val, &err_msg);
                    if (err_msg != NULL) {
                        ctx->last_error = err_msg;
                    } else {
                        ctx->last_error = NULL;
                        input_widget_highlight_off(ctx->input);
                        ctx->writing_mode = !ctx->writing_mode;
                        change_settings(ctx->settings, (SettingsOptions)ctx->input->selected, val_int);
                    }
                } else {
                    // Entering writing_mode
                    input_widget_highlight_on(ctx->input, COLOR_BLACK, COLOR_WHITE);
                    ctx->writing_mode = !ctx->writing_mode;
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

bool on_signal_settings(int fd, void* context) {
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

bool on_tick_settings(void* context) {
    app_context* ctx = (app_context*)context;
    input_widget_render(ctx->input);
    if (ctx->last_error != NULL) {
        panel_draw_text(ctx->input->panel, 1, 0, ctx->last_error, COLOR_RED, COLOR_DEFAULT);
    } 
    if (ctx->need_redraw) print_screen_buffer();
    ctx->need_redraw = false;
    return true;
}

TUIError start_settings_screen(GameSettings* settings, char** error_msg) {
    clear_screen_buffer();
    init_event_listener();
    TUIError ec = TUI_EC_Ok;
    termsize size = get_terminal_size();
    Panel panel = {.x=(size.width-30)/2, .y=(size.height-8)/2, .height=8, .width=30};
    InputWidget* widget = input_widget_init(panel);
    if (ec != TUI_EC_Ok) {*error_msg="Widget init error";input_widget_free(widget);return ec;}
    char str[20];

    snprintf(str, sizeof(str), "%d", settings->width);
    ec = input_widget_add_entry(widget, "Field width: ", 3, str);
    if (ec != TUI_EC_Ok) {*error_msg="Default field width error";input_widget_free(widget);return ec;}

    snprintf(str, sizeof(str), "%d", settings->height);
    ec = input_widget_add_entry(widget, "Field height: ", 3, str);
    if (ec != TUI_EC_Ok) {*error_msg="Default field height error";input_widget_free(widget);return ec;}

    snprintf(str, sizeof(str), "%d", settings->single_ship_count);
    ec = input_widget_add_entry(widget, "Single ship count: ", 2, str);
    if (ec != TUI_EC_Ok) {*error_msg="Default single ship count error";input_widget_free(widget);return ec;}

    snprintf(str, sizeof(str), "%d", settings->duo_ship_count);
    ec = input_widget_add_entry(widget, "Double ship count: ", 2, str);
    if (ec != TUI_EC_Ok) {*error_msg="Default double ship count error";input_widget_free(widget);return ec;}

    snprintf(str, sizeof(str), "%d", settings->triple_ship_count);
    ec = input_widget_add_entry(widget, "Triple ship count: ", 2, str);
    if (ec != TUI_EC_Ok) {*error_msg="Default triple ship count error";input_widget_free(widget);return ec;}

    snprintf(str, sizeof(str), "%d", settings->quadriple_ship_count);
    ec = input_widget_add_entry(widget, "Quadriple ship count: ", 2, str);
    if (ec != TUI_EC_Ok) {*error_msg="Default quadriple ship count error";input_widget_free(widget);return ec;}

    app_context ctx = {        
        .input = widget,
        .need_redraw = true,
        .writing_mode = false,
        .settings = settings
    };
    set_on_stdin(&on_stdin_settings);
    set_on_signal(&on_signal_settings);
    set_on_tick(&on_tick_settings);

    main_loop(&ctx);

    input_widget_free(widget);

    return TUI_EC_Ok;
}

