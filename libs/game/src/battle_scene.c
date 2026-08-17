#include "game/battle_scene.h"
#include <signal.h>
#include <sys/signalfd.h>
#include <stdio.h>
#include <string.h>
#include "game/battlefield.h"
#include "game/ai.h"
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

    TermSizeType p1_cursor_x;
    TermSizeType p1_cursor_y;

    TermSizeType p2_cursor_x;
    TermSizeType p2_cursor_y;

    Panel player1_panel;
    Panel player2_panel;

    TermSizeType board_width;
    TermSizeType board_height;

    char* last_error;

    PlayerNum turn;
    PlayerNum winner;
    bool glowing_ships;
    bool ai_enabled;
    AIMemory ai_memory;
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

    ctx->player1_panel= (Panel){ .width = panel_width, .height = panel_height, .x = x_start, .y = y_start };
    ctx->player2_panel = (Panel){ .width = panel_width, .height = panel_height, .x = x_start + panel_width + gap, .y = y_start };
}

static Panel resize_main_panel() {
    termsize size = get_terminal_size();
    Panel panel = {.height=size.height, .width=size.width, .x=0, .y=0};
    return panel;
}

static void draw_win_message(app_context* ctx) {
    char buff[64];
    int winner_num = (ctx->winner == PlayerNum_PLAYER_ONE) ? 1 : 2;
    snprintf(buff, sizeof(buff), "Player %d win! Press any key to exit", winner_num);
    TermSizeType text_len = (TermSizeType)strlen(buff);
    TermSizeType text_x = (ctx->main_panel.width > text_len) ? (ctx->main_panel.width - text_len) / 2 : 0;
    TermSizeType text_y = ctx->main_panel.height / 2;
    panel_draw_text(ctx->main_panel, text_x, text_y, buff, COLOR_GREEN, COLOR_DEFAULT);
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
            fg = COLOR_CYAN;
            c = '#';
            break;
        case CELL_TYPE_HIT:
            c = 'X';
            fg = COLOR_RED;
            break;
        case CELL_TYPE_MISS:
            fg = COLOR_YELLOW;
            c = 'O';
            break;
        case CELL_TYPE_SHIP:
            c = '#';
            fg = COLOR_CYAN;
            if (ctx->glowing_ships) {
                c = '@';
                fg = COLOR_GREEN;
            }
       break;
    }
    panel_put_char(*panel, x*2+1, y+1, c, fg, bg);
    panel_put_char(*panel, x*2+1+1, y+1, ' ', COLOR_DEFAULT, COLOR_DEFAULT);
}

static void full_redraw(app_context* ctx) {
    resize_panels(ctx);
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

static void draw_cursor(app_context* ctx) {
    Panel* panel = NULL;
    PlayerNum target = PlayerNum_PLAYER_ONE;
    if (ctx->turn == PlayerNum_PLAYER_ONE) {
        panel = &ctx->player2_panel;
        target = PlayerNum_PLAYER_TWO;
    } else {
        panel = &ctx->player1_panel;
        target = PlayerNum_PLAYER_ONE;
    }
    if (ctx->cursor_redraw) {
        draw_battlefield_cell(ctx, target, ctx->prev_cursor_x, ctx->prev_cursor_y);
        panel_put_char(*panel, ctx->cursor_x*2+1, ctx->cursor_y+1, '@', COLOR_BLACK, COLOR_WHITE);
        ctx->cursor_redraw = false;
    }
}

static void change_turn(app_context* ctx) {
    if (ctx->turn == PlayerNum_PLAYER_ONE) {
        ctx->p1_cursor_x = ctx->cursor_x;
        ctx->p1_cursor_y = ctx->cursor_y;

        ctx->cursor_x = ctx->p2_cursor_x;
        ctx->cursor_y = ctx->p2_cursor_y;

        ctx->turn = PlayerNum_PLAYER_TWO;
    } else {
        ctx->p2_cursor_x = ctx->cursor_x;
        ctx->p2_cursor_y = ctx->cursor_y;

        ctx->cursor_x = ctx->p1_cursor_x;
        ctx->cursor_y = ctx->p1_cursor_y;

        ctx->turn = PlayerNum_PLAYER_ONE;
    }
    ctx->prev_cursor_x = ctx->cursor_x;
    ctx->prev_cursor_y = ctx->cursor_y;
    if (ctx->ai_enabled && ctx->turn == PlayerNum_PLAYER_TWO) {
        ctx->cursor_redraw = false;
    } else {
        ctx->cursor_redraw = true;
    }
    ctx->need_redraw = true;
}

static void redraw_ship_area(app_context* ctx, PlayerNum owner, Ship* ship) {
    TermSizeType x = ship->x;
    TermSizeType y = ship->y;
    bool is_horizontal = ship->is_horizontal;
    TermSizeType size = (TermSizeType)ship->max_size;
 
    TermSizeType x0 = (x > 0) ? x - 1 : 0;
    TermSizeType y0 = (y > 0) ? y - 1 : 0;
    TermSizeType x1 = is_horizontal ? x + size : x + 1;
    TermSizeType y1 = is_horizontal ? y + 1 : y + size;
    x1 = (x1 < ctx->board_width) ? x1 + 1 : ctx->board_width;
    y1 = (y1 < ctx->board_height) ? y1 + 1 : ctx->board_height;
 
    for (TermSizeType cy = y0; cy < y1; ++cy) {
        for (TermSizeType cx = x0; cx < x1; ++cx) {
            draw_battlefield_cell(ctx, owner, cx, cy);
        }
    }
}

static bool shot(app_context* ctx) {
    BattleField* field = NULL;
    PlayerNum target = PlayerNum_PLAYER_ONE;
    if (ctx->turn == PlayerNum_PLAYER_ONE) {
        field = ctx->player2_field;
        target = PlayerNum_PLAYER_TWO;
    } else {
        field = ctx->player1_field;
        target = PlayerNum_PLAYER_ONE;
    }
    bool is_hit = false;
    Ship* ship = NULL;
    GameEC ec = bf_shot(field, ctx->cursor_x, ctx->cursor_y, &is_hit, &ship); 
    if (ec == GAME_EC_InternalMapError|| ec == GAME_EC_AllocationError) {ctx->critical_error = true; return false;}
    if (ec == GAME_EC_CantPlaceHere) return true;
    draw_battlefield_cell(ctx, target, ctx->cursor_x, ctx->cursor_y);
    if (ship != NULL) {
        redraw_ship_area(ctx, target, ship);
    }
    if (ctx->ai_enabled && ctx->turn == PlayerNum_PLAYER_TWO) {
        ai_set_shot_result(&ctx->ai_memory, is_hit, ship != NULL); 
    }
    if (field->ships_left == 0) {
        ctx->game_finished = true;
        ctx->winner = ctx->turn;
        ctx->need_redraw = true;
        return true;
    }
    if (!is_hit) {
        change_turn(ctx);
    }

    ctx->need_redraw = true;
    return true;
}

bool on_stdin_battle(int fd, void* cont) {
    (void)fd;
    app_context* ctx = (app_context*)cont;
    for (;;) {
        char letter = 0;
        PressedKey key = read_key(&letter);
        if (key == KEY_NOTHING) break;
        if (key == KEY_EOF) return false; 
        if (ctx->game_finished) {
            return false;
        }
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
                return shot(ctx);
                break;
            case KEY_BACKSPACE:
                break;
            case KEY_SPACE:
                break;
            case KEY_LETTER:
                if (letter == 'z') {
                    ctx->glowing_ships = !ctx->glowing_ships;
                    full_redraw(ctx);
                    ctx->cursor_redraw = true;
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

bool on_signal_battle(int fd, void* context) {
    app_context* ctx = (app_context*)context;
    struct signalfd_siginfo si;
    for (;;) {
        ssize_t r = read(fd, &si, sizeof(si));
        if (r < 0) break;
        if (r != sizeof(si)) break;
        if (si.ssi_signo == SIGINT) {
            return false;
        } else if (si.ssi_signo == SIGWINCH) {
            termsize size = get_terminal_size();
            resize_screen_buffer(size.width, size.height);
            write(STDOUT_FILENO, "\033[2J\033[H", 7);
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
    if (ctx->ai_enabled && ctx->turn == PlayerNum_PLAYER_TWO) {
        AIShot ai_shot = ai_get_next_shot(&ctx->ai_memory); 
        ctx->cursor_x = ai_shot.x;
        ctx->cursor_y = ai_shot.y;
        shot(ctx);
    }
    if (ctx->game_finished) draw_win_message(ctx);
    if (ctx->cursor_redraw) draw_cursor(ctx);
    if (ctx->need_redraw) print_screen_buffer();
    ctx->need_redraw = false;
    return true;
}

GameEC start_battle_scene(BattleField* player1_field, BattleField* player2_field, bool enable_ai) {
    clear_screen_buffer();
    init_event_listener();
    GameEC ec = GAME_EC_Ok;
    (void)ec;

    app_context ctx = {        
        .turn = PlayerNum_PLAYER_ONE,
        .need_redraw = true,
        .main_panel = resize_main_panel(),
        .cursor_x = 0,
        .cursor_y = 0,
        .cursor_redraw = true,
        .critical_error = false,
        .player1_field = player1_field,
        .player2_field = player2_field,
        .board_width = player1_field->width,
        .board_height = player1_field->height,
        .glowing_ships = false,
        .ai_enabled = enable_ai,
    };
    if (ctx.ai_enabled) {
        ctx.ai_memory = (AIMemory){
           .bf = player1_field,
           .is_searching = true,
           .cant_find_valid_shot = false,
           .last_shot = (LastShot){
               .found_correct_axis = false,
           }
        };
    }
    
    set_on_stdin(&on_stdin_battle);
    set_on_signal(&on_signal_battle);
    set_on_tick(&on_tick_battle);

    resize_panels(&ctx);
    full_redraw(&ctx);
    main_loop(&ctx);
    
    if (ctx.critical_error) return GAME_EC_InternalMapError;

    return GAME_EC_Ok;
}
