#include "zxcurses/screen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char symbol;
    Color fg;
    Color bg;
} Cell;

static Cell* g_screen_buffer;
static TermSizeType g_width;
static TermSizeType g_height;

static void fill_buffer(Cell* buffer, size_t size) {
    Cell c = {.symbol = ' ', .fg = 0, .bg = 0};
    for (size_t i = 0; i < size; ++i) {
        buffer[i] = c;
    }
}

TUIError create_screen_buffer(TermSizeType width, TermSizeType height) {
    g_screen_buffer = calloc(height * width, sizeof(Cell));
    if (g_screen_buffer == NULL) return TUI_EC_AllocationError;
    fill_buffer(g_screen_buffer, height * width);
    g_width = width;
    g_height = height;
    return TUI_EC_Ok;
}

TUIError resize_screen_buffer(TermSizeType width, TermSizeType height) {
    free(g_screen_buffer);
    g_screen_buffer = calloc(height * width, sizeof(Cell));
    if (g_screen_buffer == NULL) return TUI_EC_AllocationError;
    fill_buffer(g_screen_buffer, height * width);
    g_width = width;
    g_height = height;
    return TUI_EC_Ok;
}

void clear_screen_buffer() { fill_buffer(g_screen_buffer, g_height * g_width); }

void print_screen_buffer() {
    for (TermSizeType i = 0; i < g_height; ++i) {
        for (TermSizeType j = 0; j < g_width; ++j) {
            move_cursor(j + 1, i + 1);
            Cell* c = &g_screen_buffer[i * g_width + j];
            int fg_code = color_fg_ansi_code(c->fg);
            int bg_code = color_bg_ansi_code(c->bg);
            char buf[64];
            int n = snprintf(buf, sizeof(buf), "\x1b[%d;%dm", fg_code, bg_code);
            write(STDOUT_FILENO, buf, (size_t)n);
            write(STDOUT_FILENO, &c->symbol, 1);
        }
    }
    write(STDOUT_FILENO, "\x1b[0m", 4);
}

TUIError draw_text(TermSizeType x, TermSizeType y, char* text, Color fg,
                   Color bg) {
    if (g_height <= y) return TUI_EC_OutOfBounds;
    if (g_width <= x + strlen(text)) return TUI_EC_OutOfBounds;
    for (TermSizeType i = 0; text[i] != '\0'; ++i) {
        g_screen_buffer[g_width * y + x + i] =
            (Cell){.symbol = text[i], .fg = fg, .bg = bg};
    }
    return TUI_EC_Ok;
}

TUIError set_cell(TermSizeType x, TermSizeType y, char ch, Color fg, Color bg) {
    if (x >= g_width || y >= g_height) return TUI_EC_OutOfBounds;
    g_screen_buffer[g_width * y + x].symbol = ch;
    set_color(x, y, fg, bg);
    return TUI_EC_Ok;
}

void set_color(TermSizeType x, TermSizeType y, Color fg, Color bg) {
    g_screen_buffer[g_width * y + x].fg = fg;
    g_screen_buffer[g_width * y + x].bg = bg;
}

void fill_color(TermSizeType x1, TermSizeType y1, TermSizeType x2,
                TermSizeType y2, Color fg, Color bg) {
    if (x1 > g_width || x2 > g_width || y1 > g_height || y2 > g_height) return;
}
