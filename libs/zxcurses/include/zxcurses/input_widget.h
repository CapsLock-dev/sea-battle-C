#ifndef CL_ZXCURSES_INPUT_WIDGET_H
#define CL_ZXCURSES_INPUT_WIDGET_H
#include "zxcurses/error.h"
#include "zxcurses/panel.h"

typedef struct {
    char* name;
    char* value;
    size_t symbol_limit;
    size_t curr_symbol_count;
} InputEntry;

typedef struct {
    Panel panel;
    InputEntry* entries;
    size_t curr_entries;
    size_t selected;
    Color highlight_fg;
    Color highlight_bg;
    bool highlight_on;
} InputWidget;

InputWidget* input_widget_init(Panel panel);
TUIError input_widget_add_entry(InputWidget* widget, const char* label,
                                size_t limit, char* base_value);
void input_widget_free(InputWidget* widget);

void input_widget_render(InputWidget* widget);
void input_widget_move_selection(InputWidget* widget, int direction);
void input_widget_insert_symbol(InputWidget* widget, char symbol);
void input_widget_remove_symbol(InputWidget* widget);
void input_widget_highlight_on(InputWidget* widget, Color fg, Color bg);
void input_widget_highlight_off(InputWidget* widget);

#endif
