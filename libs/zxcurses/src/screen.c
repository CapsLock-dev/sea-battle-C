#include "zxcurses/screen.h"
#include <stdlib.h>
#include <string.h>

static char* g_screen_buffer;
static TermSizeType g_width;
static TermSizeType g_height;

bool create_screen_buffer(TermSizeType width, TermSizeType height) {
    g_screen_buffer = calloc(height*width, sizeof(char));
    if (g_screen_buffer == NULL) return false;
    g_width = width;
    g_height = height;
    memset(g_screen_buffer, '#', height*width);
    return true;
}

bool resize_screen_buffer(TermSizeType width, TermSizeType height) {
    free(g_screen_buffer);
    g_screen_buffer = calloc(height*width, sizeof(char));
    if (g_screen_buffer == NULL) return false;
    g_width = width;
    g_height = height;
    memset(g_screen_buffer, '#', height*width);
    return true;
}

void print_screen_buffer() {
    for (TermSizeType i=0; i<g_height; ++i) {
        for (TermSizeType j=0; j<g_width; ++j) {
            move_cursor(j+1,i+1); 
            write(STDOUT_FILENO, &g_screen_buffer[i*g_width+j], 1);
        }
    }
}

void draw_text(TermSizeType x, TermSizeType y, char* text) {
    if (g_height <= y) return;
    if (g_width <= x+strlen(text)) return;
    for (TermSizeType i=0; text[i]!='\0'; ++i) {
        g_screen_buffer[g_width*y+x+i] = text[i];
    }
}

void set_cell(TermSizeType x, TermSizeType y, char ch) {
    if (x >= g_width || y >= g_height) return;
    g_screen_buffer[g_width * y + x] = ch;
}
