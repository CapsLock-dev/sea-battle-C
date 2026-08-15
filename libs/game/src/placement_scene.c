#include <signal.h>
#include <sys/signalfd.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include "game/placement_scene.h"
#include "game/battlefield.h"
#include "zxcurses/panel.h"
#include "zxcurses/screen.h"
#include "zxcurses/event_listener.h"

typedef struct {
    bool need_redraw;
    GameSettings settings;
    Panel panel;
    Panel main_panel;
    TermSizeType cursor_x;
    TermSizeType cursor_y;

    TermSizeType prev_cursor_x;
    TermSizeType prev_cursor_y;
    bool cursor_redraw;

    bool placing_mode;
    ShipType curr_ship_type;
    bool is_horizontal;
    BattleField* field;

    char* last_error;
} app_context;

static Panel resize_panel(GameSettings settings) {
    termsize size = get_terminal_size();
    settings.width = settings.width*2;
    Panel panel = {.height=settings.height+2, .width=settings.width+2, .x=(size.width-settings.width)/2, .y=(size.height-settings.height)/2};
    return panel;
}

static Panel resize_main_panel() {
    termsize size = get_terminal_size();
    Panel panel = {.height=size.height, .width=size.width, .x=0, .y=0};
    return panel;
}

static TermSizeType get_ship_count(const app_context* ctx, ShipType type) {
    switch (type) {
        case SHIP_TYPE_SIGNLE:
            return ctx->settings.single_ship_count;
        case SHIP_TYPE_DOUBLE:
            return ctx->settings.duo_ship_count;
        case SHIP_TYPE_TRIPLE:
            return ctx->settings.triple_ship_count;
        case SHIP_TYPE_QUADRIPLE:
            return ctx->settings.quadriple_ship_count;
    }
}

static void draw_battlefield_cell(const app_context* ctx, TermSizeType x, TermSizeType y) {
    CellType ct = bf_get_cell(ctx->field, x, y);
    Color fg = COLOR_DEFAULT;
    Color bg = COLOR_DEFAULT;
    char c = ' ';
    switch (ct) {
        case CELL_TYPE_EMPTY:
            c = ' ';
            break;
        case CELL_TYPE_HIT:
            c = 'X';
            break;
        case CELL_TYPE_MISS:
            c = 'O';
            break;
        case CELL_TYPE_SHIP:
            c = '@';
            break;
    }
    panel_put_char(ctx->panel, x*2+1, y+1, c, fg, bg);
}

static void full_redraw(const app_context* ctx) {
    for (TermSizeType y = 1; y < ctx->panel.height - 1; ++y) {
        for (TermSizeType x = 1; x < ctx->panel.width - 1; x += 2) {
            int game_col = (x - 1) / 2;
            int game_row = y - 1; 
            if (game_row == ctx->cursor_y && game_col == ctx->cursor_x) {
                panel_put_char(ctx->panel, x, y, '@', COLOR_BLACK, COLOR_WHITE);
            } else {
                panel_put_char(ctx->panel, x, y, '#', COLOR_CYAN, COLOR_DEFAULT);
                panel_put_char(ctx->panel, x+1, y, ' ', COLOR_DEFAULT, COLOR_DEFAULT);
            }
        }
    }
    char buff[50];
    snprintf(buff, sizeof(buff), "Current ship size: %u", ctx->curr_ship_type+1);
    panel_draw_text(ctx->main_panel, 1,1, buff, COLOR_DEFAULT, COLOR_DEFAULT);
    snprintf(buff, sizeof(buff), "Ship count: %u", get_ship_count(ctx, ctx->curr_ship_type));
    panel_draw_text(ctx->main_panel, 1,2, buff, COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_text(ctx->main_panel, 1,3, "Press ARROW KEYS to move", COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_text(ctx->main_panel, 1,4, "Press ENTER to preview placing, press second time to place", COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_text(ctx->main_panel, 1,5, "Press BACKSPACE to leave preview mode", COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_text(ctx->main_panel, 1,6, "Press SPACE to rotate while in preview mode", COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_box(ctx->panel, COLOR_DEFAULT, COLOR_DEFAULT);
}

static void draw_placing_mode(const app_context* ctx) {
    if (ctx->placing_mode) {
        TermSizeType origin_row = ctx->cursor_y;
        TermSizeType origin_col = ctx->cursor_x;
        for (TermSizeType i=0; i<ctx->curr_ship_type+1; ++i) {
            TermSizeType seg_row = origin_row;
            TermSizeType seg_col = origin_col;
            if (ctx->is_horizontal) {
                seg_col += i;
            } else {
                seg_row += i;
            }
            TermSizeType panel_x = 1 + (seg_col * 2);
            TermSizeType panel_y = 1 + seg_row;
            panel_put_char(ctx->panel, panel_x, panel_y, '@', COLOR_BLACK, COLOR_WHITE);
            panel_put_char(ctx->panel, panel_x+1, panel_y, ' ', COLOR_DEFAULT, COLOR_DEFAULT);
        }
    }
}

bool on_stdin_placement(int fd, void* cont) {
    (void)fd;
    app_context* ctx = (app_context*)cont;
    for (;;) {
        char letter = 0;
        PressedKey key = read_key(&letter);
        if (key == KEY_NOTHING) break;
        if (key == KEY_EOF) return false;
        switch (key) {
            case KEY_UP:
                if (ctx->placing_mode) break;
                if (ctx->cursor_y > 0) {
                    ctx->prev_cursor_y = ctx->cursor_y;
                    ctx->cursor_y--;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_DOWN:
                if (ctx->placing_mode) break;
                if (ctx->cursor_y < ctx->settings.height - 1) {
                    ctx->prev_cursor_y = ctx->cursor_y;
                    ctx->cursor_y++;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_LEFT:
                if (ctx->placing_mode) break;
                if (ctx->cursor_x > 0){
                    ctx->prev_cursor_x = ctx->cursor_x;
                    ctx->cursor_x--;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_RIGHT:
                if (ctx->placing_mode) break;
                if (ctx->cursor_x < ctx->settings.width - 1) {
                    ctx->prev_cursor_x = ctx->cursor_x;
                    ctx->cursor_x++;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_ENTER:
                if (ctx->placing_mode) {
                    bool res = bf_place_ship(ctx->field, ctx->cursor_x, ctx->cursor_y, ctx->is_horizontal, ctx->curr_ship_type);
                    if (!res) {
                        ctx->last_error = "Can't place ship here";
                    }
                }
                ctx->placing_mode = true;
                break;
            case KEY_BACKSPACE:
                ctx->placing_mode = false;
                ctx->last_error = NULL;
                break;
            case KEY_SPACE:
                ctx->is_horizontal = !ctx->is_horizontal;
                break;
            default:
                break;
        }
        ctx->need_redraw = true;
        break;
    }
    return true;
}

bool on_signal_placement(int fd, void* context) {
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
            ctx->panel = resize_panel(ctx->settings);
            ctx->main_panel = resize_main_panel();
            full_redraw(ctx);
            ctx->need_redraw = true;
        }
    }
    return true;
}

bool on_tick_placement(void* context) {
    app_context* ctx = (app_context*)context;
    panel_draw_box(ctx->main_panel, COLOR_DEFAULT, COLOR_DEFAULT);
    if(ctx->last_error != NULL) panel_draw_text(ctx->main_panel, (ctx->main_panel.width-strlen(ctx->last_error))/2, 0, ctx->last_error, COLOR_RED, COLOR_DEFAULT);
    if (ctx->cursor_redraw) {
        draw_battlefield_cell(ctx, ctx->prev_cursor_x, ctx->prev_cursor_x);
        panel_put_char(ctx->panel, ctx->cursor_x*2+1, ctx->cursor_y+1, '@', COLOR_DEFAULT, COLOR_DEFAULT);
        ctx->cursor_redraw = false;
    }
    if (ctx->need_redraw) print_screen_buffer();
    ctx->need_redraw = false;
    return true;
}

TUIError start_placement_screen(GameSettings settings, BattleField* field) {
    clear_screen_buffer();
    init_event_listener();
    TUIError ec = TUI_EC_Ok;
    (void)ec;

    app_context ctx = {        
        .need_redraw = true,
        .settings = settings,
        .panel = resize_panel(settings),
        .main_panel = resize_main_panel(),
        .field = field,
        .cursor_x = 0,
        .cursor_y = 0,
        .placing_mode = false,
        .is_horizontal = false,
    };
    
    if (settings.quadriple_ship_count > 0) {
        ctx.curr_ship_type = SHIP_TYPE_QUADRIPLE;
    } else if (settings.triple_ship_count > 0) {
        ctx.curr_ship_type = SHIP_TYPE_TRIPLE;
    } else if (settings.duo_ship_count > 0) {
        ctx.curr_ship_type = SHIP_TYPE_DOUBLE;
    } else if (settings.single_ship_count > 0) {
        ctx.curr_ship_type = SHIP_TYPE_SIGNLE;
    } else {
        // Should be impossible, settings_screen requires non zero values
        return TUI_EC_Ok; 
    }

    set_on_stdin(&on_stdin_placement);
    set_on_signal(&on_signal_placement);
    set_on_tick(&on_tick_placement);

    full_redraw(&ctx);
    main_loop(&ctx);

    return TUI_EC_Ok;
}

