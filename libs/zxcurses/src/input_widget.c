#include "zxcurses/input_widget.h"
#include <stdlib.h>
#include <string.h>

InputWidget* input_widget_init(Panel panel) {
    InputWidget* widget = calloc(1, sizeof(InputWidget));
    if (widget == NULL) return NULL;
    widget->panel = panel;
    return widget;
}

TUIError input_widget_add_entry(InputWidget* widget, const char* label, size_t limit, char* base_value) {
    if (widget == NULL) return TUI_EC_IsNull;
    if (strlen(base_value) > limit) return TUI_EC_OutOfBounds;

    InputEntry* temp = realloc(widget->entries, sizeof(InputEntry)*(widget->curr_entries+1));
    if (temp == NULL) return TUI_EC_AllocationError;
    widget->entries = temp;

    if (widget->entries == NULL) return TUI_EC_AllocationError;
    InputEntry* entry = &widget->entries[widget->curr_entries];

    entry->name = calloc(strlen(label)+1, sizeof(char));
    if (entry->name == NULL) return TUI_EC_AllocationError;
    strcpy(entry->name, label);

    entry->value = calloc(limit+1, sizeof(char));
    if (entry->value == NULL) return TUI_EC_AllocationError;
    strcpy(entry->value, base_value);
    entry->curr_symbol_count = strlen(base_value);
    entry->symbol_limit = limit;
    ++widget->curr_entries;

    return TUI_EC_Ok;
}

void input_widget_remove_symbol(InputWidget* widget) {
    if (widget == NULL) return;
    InputEntry* entry = &widget->entries[widget->selected];
    if (entry->curr_symbol_count == 0) return;
    entry->value[--entry->curr_symbol_count] = ' ';
}

void input_widget_free(InputWidget* widget) {
    if (widget == NULL) return;
    for (size_t i=0; i<widget->curr_entries; ++i) {
        free(widget->entries[i].value);
        free(widget->entries[i].name);
    }
    free(widget->entries);
    free(widget);
}

void input_widget_render(InputWidget* widget) {
    if (widget == NULL) return;
    Color fg = COLOR_DEFAULT;
    Color bg = COLOR_DEFAULT;
    if (widget->highlight_on) {
        fg = widget->highlight_fg;
        bg = widget->highlight_bg;
    }
    for (size_t i=0; i<widget->curr_entries; ++i) {
        if(widget->selected == i) {
            panel_draw_text(widget->panel, 1, (TermSizeType)i+1, ">", COLOR_DEFAULT, COLOR_DEFAULT);
            panel_draw_text(widget->panel, 2+(TermSizeType)strlen(widget->entries[i].name), (TermSizeType)i+1, widget->entries[i].value, fg, bg);
        } else {
            panel_draw_text(widget->panel, 1, (TermSizeType)i+1, "|", COLOR_DEFAULT, COLOR_DEFAULT);
            panel_draw_text(widget->panel, 2+(TermSizeType)strlen(widget->entries[i].name), (TermSizeType)i+1, widget->entries[i].value, COLOR_DEFAULT, COLOR_DEFAULT);
        }
        panel_draw_text(widget->panel, 2, (TermSizeType)i+1, widget->entries[i].name, COLOR_DEFAULT, COLOR_DEFAULT);

    }
    panel_draw_box(widget->panel, COLOR_DEFAULT, COLOR_DEFAULT);
}

void input_widget_move_selection(InputWidget* widget, int direction) {
    if (widget == NULL || direction == 0 || widget->curr_entries == 0) return;
    if (direction < 0) {
        widget->selected = (widget->selected + 1) % (widget->curr_entries);
    } else if (direction > 0){
        if (widget->selected == 0) {
            widget->selected = widget->curr_entries-1;
        } else {
            --widget->selected;
        }
    }
}

void input_widget_insert_symbol(InputWidget* widget, char symbol) {
    if (widget == NULL) return;
    InputEntry* curr_entry = &widget->entries[widget->selected];
    if (curr_entry->curr_symbol_count >= curr_entry->symbol_limit) return;
    curr_entry->value[curr_entry->curr_symbol_count++] = symbol;
}

void input_widget_highlight_on(InputWidget* widget, Color fg, Color bg) {
    widget->highlight_bg = bg;
    widget->highlight_fg = fg;
    widget->highlight_on = true;
}

void input_widget_highlight_off(InputWidget* widget) {
    widget->highlight_on = false;
}
