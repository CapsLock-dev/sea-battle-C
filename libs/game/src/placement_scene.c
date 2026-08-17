#include <signal.h>
#include <sys/signalfd.h>
#include <stdio.h>
#include <string.h>
#include "game/placement_scene.h"
#include "game/battlefield.h"
#include "zxcurses/panel.h"
#include "zxcurses/screen.h"
#include "zxcurses/event_listener.h"

typedef struct {
    bool placement_finished;
    bool critical_error;
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
    TermSizeType curr_ship_count;

    bool is_horizontal;
    BattleField* field;
    bool placing_redraw;

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

static void draw_battlefield_cell(app_context* ctx, TermSizeType x, TermSizeType y) {
    if (x >= ctx->field->width || y >= ctx->field->height) return;
    CellType ct = bf_get_cell(ctx->field, x, y);
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
            fg = COLOR_GREEN;
            c = '@';
            break;
    }
    panel_put_char(ctx->panel, x*2+1, y+1, c, fg, bg);
    panel_put_char(ctx->panel, x*2+1+1, y+1, ' ', COLOR_DEFAULT, COLOR_DEFAULT);
}

static void draw_ship_count(app_context* ctx) {
    char buff[50];
    snprintf(buff, sizeof(buff), "Current ship size: %u", ctx->curr_ship_type+1);
    panel_draw_text(ctx->main_panel, 1,1, buff, COLOR_DEFAULT, COLOR_DEFAULT);
    snprintf(buff, sizeof(buff), "Ship count: %u", ctx->curr_ship_count);
    panel_draw_text(ctx->main_panel, 1,2, buff, COLOR_DEFAULT, COLOR_DEFAULT);
}

static void full_redraw(app_context* ctx) {
    for (TermSizeType x=0; x<ctx->field->width; ++x) {
        for (TermSizeType y=0; y<ctx->field->height; ++y) {
            draw_battlefield_cell(ctx, x, y);
        }
    }
    draw_ship_count(ctx);
    panel_draw_text(ctx->main_panel, 1,3, "Press ARROW KEYS to move", COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_text(ctx->main_panel, 1,4, "Press ENTER to preview placing, press second time to place", COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_text(ctx->main_panel, 1,5, "Press BACKSPACE to leave preview mode", COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_text(ctx->main_panel, 1,6, "Press SPACE to rotate while in preview mode", COLOR_DEFAULT, COLOR_DEFAULT);
    panel_draw_box(ctx->panel, COLOR_DEFAULT, COLOR_DEFAULT);
}

static void draw_placing_mode(app_context* ctx) {
    TermSizeType cur_y = ctx->cursor_y;
    TermSizeType cur_x = ctx->cursor_x;
    for (TermSizeType i=0; i<ctx->curr_ship_type+1; ++i) {
        TermSizeType field_x = cur_x;
        TermSizeType field_y = cur_y;
        if (ctx->is_horizontal) {
            field_x += i; 
        } else {
            field_y += i;
        }
        TermSizeType panel_x = field_x*2+1;
        TermSizeType panel_y = field_y+1;
        if (ctx->is_horizontal) {
            if (i != 0) draw_battlefield_cell(ctx, (cur_x), (cur_y+i)); 
        } else {
            if (i != 0) draw_battlefield_cell(ctx, (cur_x+i), cur_y); 
        }
        if (ctx->placing_mode && panel_x+1 < ctx->panel.width && panel_y+1 < ctx->panel.height) {
            panel_put_char(ctx->panel, panel_x, panel_y, '@', COLOR_BLACK, COLOR_WHITE);
            panel_put_char(ctx->panel, panel_x+1, panel_y, ' ', COLOR_DEFAULT, COLOR_DEFAULT);
        } else {
            if (i != 0) {
                draw_battlefield_cell(ctx, (cur_x+i), cur_y); 
                draw_battlefield_cell(ctx, (cur_x), (cur_y+i)); 
            }
        }
    }
}

static bool after_place_ship(app_context* ctx) {
    for (TermSizeType i=0; i<ctx->curr_ship_type+1; ++i) {
        TermSizeType cx = ctx->cursor_x+i*ctx->is_horizontal;
        TermSizeType cy = ctx->cursor_y+i*(!ctx->is_horizontal);
        draw_battlefield_cell(ctx, cx, cy);
    }
    --(ctx->curr_ship_count);
    if (ctx->curr_ship_count == 0) {
        switch (ctx->curr_ship_type) {
            case SHIP_TYPE_SIGNLE:
                ctx->placement_finished = true;
                return false;
                break;
            case SHIP_TYPE_DOUBLE:
                ctx->curr_ship_type = SHIP_TYPE_SIGNLE;
                ctx->curr_ship_count = ctx->settings.single_ship_count;
                break;
            case SHIP_TYPE_TRIPLE:
                ctx->curr_ship_type = SHIP_TYPE_DOUBLE;
                ctx->curr_ship_count = ctx->settings.duo_ship_count;
                break;
            case SHIP_TYPE_QUADRIPLE:
                ctx->curr_ship_type = SHIP_TYPE_TRIPLE;
                ctx->curr_ship_count = ctx->settings.triple_ship_count;
                break;
        }
    }
    draw_ship_count(ctx);
    return true;
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
                    ctx->prev_cursor_x = ctx->cursor_x;
                    ctx->prev_cursor_y = ctx->cursor_y;
                    ctx->cursor_y--;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_DOWN:
                if (ctx->placing_mode) break;
                if (ctx->cursor_y < ctx->settings.height - 1) {
                    ctx->prev_cursor_x = ctx->cursor_x;
                    ctx->prev_cursor_y = ctx->cursor_y;
                    ctx->cursor_y++;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_LEFT:
                if (ctx->placing_mode) break;
                if (ctx->cursor_x > 0){
                    ctx->prev_cursor_x = ctx->cursor_x;
                    ctx->prev_cursor_y = ctx->cursor_y;
                    ctx->cursor_x--;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_RIGHT:
                if (ctx->placing_mode) break;
                if (ctx->cursor_x < ctx->settings.width - 1) {
                    ctx->prev_cursor_x = ctx->cursor_x;
                    ctx->prev_cursor_y = ctx->cursor_y;
                    ctx->cursor_x++;
                }
                ctx->cursor_redraw = true;
                break;
            case KEY_ENTER:
                if (ctx->placing_mode) {
                    GameEC res = bf_place_ship(ctx->field, ctx->cursor_x, ctx->cursor_y, ctx->is_horizontal, ctx->curr_ship_type);
                    if (res != GAME_EC_Ok) {
                        if (res == GAME_EC_InternalMapError || res == GAME_EC_AllocationError) {ctx->critical_error = true; return false;}
                        ctx->last_error = "Can't place ship here";
                    } else {
                        ctx->placing_mode = false;
                        ctx->placing_redraw = true;
                        ctx->need_redraw = true;
                        return after_place_ship(ctx);
                    }
                }
                ctx->placing_mode = true;
                ctx->placing_redraw = true;
                break;
            case KEY_BACKSPACE:
                ctx->placing_redraw = true;
                ctx->placing_mode = false;
                ctx->last_error = NULL;
                break;
            case KEY_SPACE:
                if (ctx->placing_mode) {
                    ctx->placing_redraw = true;
                    ctx->is_horizontal = !ctx->is_horizontal;
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
            ctx->cursor_redraw = true;
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
        draw_battlefield_cell(ctx, ctx->prev_cursor_x, ctx->prev_cursor_y);
        panel_put_char(ctx->panel, ctx->cursor_x*2+1, ctx->cursor_y+1, '@', COLOR_BLACK, COLOR_WHITE);
        ctx->cursor_redraw = false;
    }
    if (ctx->placing_redraw) {
        draw_placing_mode(ctx);
        ctx->placing_redraw = false;
    }
    if (ctx->need_redraw) print_screen_buffer();
    ctx->need_redraw = false;
    return true;
}

GameEC start_placement_screen(GameSettings settings, BattleField* field) {
    clear_screen_buffer();
    init_event_listener();
    GameEC ec = GAME_EC_Ok;
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
        .cursor_redraw = true,
        .placing_redraw = false,
        .placement_finished = false,
        .critical_error = false,
    };
    
    if (settings.quadriple_ship_count > 0) {
        ctx.curr_ship_type = SHIP_TYPE_QUADRIPLE;
        ctx.curr_ship_count = settings.quadriple_ship_count;
    } else if (settings.triple_ship_count > 0) {
        ctx.curr_ship_type = SHIP_TYPE_TRIPLE;
        ctx.curr_ship_count = settings.triple_ship_count;
    } else if (settings.duo_ship_count > 0) {
        ctx.curr_ship_type = SHIP_TYPE_DOUBLE;
        ctx.curr_ship_count = settings.duo_ship_count;
    } else if (settings.single_ship_count > 0) {
        ctx.curr_ship_type = SHIP_TYPE_SIGNLE;
        ctx.curr_ship_count = settings.single_ship_count;
    } else {
        // Should be impossible, settings_screen requires non zero values
        return GAME_EC_Ok; 
    }

    set_on_stdin(&on_stdin_placement);
    set_on_signal(&on_signal_placement);
    set_on_tick(&on_tick_placement);

    full_redraw(&ctx);
    main_loop(&ctx);
    
    if (ctx.placement_finished) return GAME_EC_PlacementFinished;
    if (ctx.critical_error) return GAME_EC_InternalMapError;

    return GAME_EC_Ok;
}

