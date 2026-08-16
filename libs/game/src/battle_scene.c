#include "game/battle_scene.h"
#include <signal.h>
#include <sys/signalfd.h>
#include <stdio.h>
#include <string.h>
#include "game/battlefield.h"
#include "zxcurses/panel.h"
#include "zxcurses/screen.h"
#include "zxcurses/event_listener.h"

typedef enum {
    PlayerNum_PLAYER_ONE,
    PlayerNum_PLAYER_TWO,
} PlayerNum;

typedef struct {
    bool game_finished;
    bool critical_error;
    bool need_redraw;
    Panel main_panel;
    TermSizeType cursor_x;
    TermSizeType cursor_y;

    TermSizeType prev_cursor_x;
    TermSizeType prev_cursor_y;
    bool cursor_redraw;

    BattleField* player1_field;
    BattleField* player2_field;
    Panel player1_panel;
    Panel player2_panel;

    TermSizeType board_width;
    TermSizeType board_height;

    bool shot_redraw;

    char* last_error;
} app_context;

static void resize_panels(app_context* ctx) {
    termsize size = get_terminal_size();

    TermSizeType board_width_chars=ctx->board_width*2;
    TermSizeType board_height=ctx->board_height;

    TermSizeType panel_width = board_width_chars+2;
    TermSizeType panel_height = board_height+2;

    TermSizeType gap = 2;
    TermSizeType total_width = 2 * panel_width + gap;
    TermSizeType total_height = panel_height;

    TermSizeType x_start = (size.width  - total_width) / 2;
    TermSizeType y_start = (size.height - total_height) / 2;

    ctx->player1_panel= (Panel){ .width = panel_width, .height = panel_height,
                        .x = x_start, .y = y_start };
    ctx->player2_panel = (Panel){ .width = panel_width, .height = panel_height,
                        .x = x_start + panel_width + gap, .y = y_start };
}

static Panel resize_main_panel() {
    termsize size = get_terminal_size();
    Panel panel = {.height=size.height, .width=size.width, .x=0, .y=0};
    return panel;
}

static void draw_battlefield_cell(app_context* ctx, PlayerNum player_num, TermSizeType x, TermSizeType y) {
    if (x >= ctx->board_width || y >= ctx->board_height) return;
    Panel* panel = NULL;
    BattleField* field = NULL;
    if (player_num == PlayerNum_PLAYER_ONE) {
        panel = &ctx->player1_panel;
        field = ctx->player1_field;
    } else {
        panel = &ctx->player2_panel;
        field = ctx->player2_field;
    }
    CellType ct = bf_get_cell(field, x, y);
    Color fg = COLOR_DEFAULT;
    Color bg = COLOR_DEFAULT;
    char c = ' ';
    switch (ct) {
        case CELL_TYPE_RADIUS:
        case CELL_TYPE_EMPTY:
            c = '#';
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
    panel_put_char(*panel, x*2+1, y+1, c, fg, bg);
    panel_put_char(*panel, x*2+1+1, y+1, ' ', COLOR_DEFAULT, COLOR_DEFAULT);
}

static void full_redraw(app_context* ctx) {
    for (TermSizeType x=0; x<ctx->board_width; ++x) {
        for (TermSizeType y=0; y<ctx->board_height; ++y) {
            draw_battlefield_cell(ctx, PlayerNum_PLAYER_ONE, x, y);
            draw_battlefield_cell(ctx, PlayerNum_PLAYER_TWO, x, y);
        }
    }
    panel_draw_text(ctx->main_panel, 1,1, "Press ARROW KEYS to move", COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_text(ctx->main_panel, 1,2, "Press ENTER to short", COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_box(ctx->player1_panel, COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_box(ctx->player2_panel, COLOR_DEFAULT, COLOR_DEFAULT);
}

bool on_stdin_battle(int fd, void* cont) {
    (void)fd;
    app_context* ctx = (app_context*)cont;
    for (;;) {
        char letter = 0;
        PressedKey key = read_key(&letter);
        if (key == KEY_NOTHING) break;
        if (key == KEY_EOF) return false;
        switch (key) {
            case KEY_UP:
                if (ctx->cursor_y > 0) {
                    ctx->prev_cursor_x = ctx->cursor_x;
                    ctx->prev_cursor_y = ctx->cursor_y;
                    ctx->cursor_y--;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_DOWN:
                if (ctx->cursor_y < ctx->board_height - 1) {
                    ctx->prev_cursor_x = ctx->cursor_x;
                    ctx->prev_cursor_y = ctx->cursor_y;
                    ctx->cursor_y++;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_LEFT:
                if (ctx->cursor_x > 0){
                    ctx->prev_cursor_x = ctx->cursor_x;
                    ctx->prev_cursor_y = ctx->cursor_y;
                    ctx->cursor_x--;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_RIGHT:
                if (ctx->cursor_x < ctx->board_width - 1) {
                    ctx->prev_cursor_x = ctx->cursor_x;
                    ctx->prev_cursor_y = ctx->cursor_y;
                    ctx->cursor_x++;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_ENTER:
                break;
            case KEY_BACKSPACE:
                break;
            case KEY_SPACE:
                break;
            default:
                break;
        }
        ctx->need_redraw = true;
        break;
    }
    return true;
}

bool on_signal_battle(int fd, void* context) {
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
            resize_panels(ctx);
            ctx->main_panel = resize_main_panel();
            full_redraw(ctx);
            ctx->cursor_redraw = true;
            ctx->need_redraw = true;
        }
    }
    return true;
}

bool on_tick_battle(void* context) {
    app_context* ctx = (app_context*)context;
    panel_draw_box(ctx->main_panel, COLOR_DEFAULT, COLOR_DEFAULT);
    if(ctx->last_error != NULL) panel_draw_text(ctx->main_panel, (ctx->main_panel.width-strlen(ctx->last_error))/2, 0, ctx->last_error, COLOR_RED, COLOR_DEFAULT);
    if (ctx->need_redraw) print_screen_buffer();
    ctx->need_redraw = false;
    return true;
}

GameEC start_battle_scene(BattleField* player1_field, BattleField* player2_field) {
    clear_screen_buffer();
    init_event_listener();
    GameEC ec = GAME_EC_Ok;
    (void)ec;

    app_context ctx = {        
        .need_redraw = true,
        .main_panel = resize_main_panel(),
        .cursor_x = 0,
        .cursor_y = 0,
        .cursor_redraw = true,
        .critical_error = false,
        .player1_field = player1_field,
        .player2_field = player2_field
    };
    
    set_on_stdin(&on_stdin_battle);
    set_on_signal(&on_signal_battle);
    set_on_tick(&on_tick_battle);

    full_redraw(&ctx);
    main_loop(&ctx);
    
    if (ctx.critical_error) return GAME_EC_InternalMapError;

    return GAME_EC_Ok;
}

