#include "zxcurses/panel.h"
#include "zxcurses/screen.h"
#include <string.h>

void panel_put_char(Panel panel, TermSizeType relative_x, TermSizeType relative_y, char ch) {
    if (relative_x >= panel.width || relative_y >= panel.height) return;
    set_cell(panel.x + relative_x, panel.y + relative_y, ch);
}

void panel_draw_text(Panel panel, TermSizeType lx, TermSizeType ly, const char* text) {
    if (ly >= panel.height) return;
    size_t len = strlen(text);
    for (size_t i = 0; i < len; i++) {
        TermSizeType px = (TermSizeType)(lx + i);
        if (px >= panel.width) break;
        panel_put_char(panel, px, ly, text[i]);
    }
}

void panel_draw_box(Panel panel) {
    if (panel.width < 2 || panel.height < 2) return;
    for (TermSizeType x = 0; x < panel.width; x++) {
        panel_put_char(panel, x, 0, '-');
        panel_put_char(panel, x, (TermSizeType)(panel.height - 1), '-');
    }
    for (TermSizeType y = 0; y < panel.height; y++) {
        panel_put_char(panel, 0, y, '|');
        panel_put_char(panel, (TermSizeType)(panel.width - 1), y, '|');
    }
    panel_put_char(panel, 0, 0, '+');
    panel_put_char(panel, (TermSizeType)(panel.width - 1), 0, '+');
    panel_put_char(panel, 0, (TermSizeType)(panel.height - 1), '+');
    panel_put_char(panel, (TermSizeType)(panel.width - 1), (TermSizeType)(panel.height - 1), '+');
}

void panel_fill(Panel panel, char ch) {
    for (TermSizeType y = 0; y < panel.height; y++) {
        for (TermSizeType x = 0; x < panel.width; x++) {
            panel_put_char(panel, x, y, ch);
        }
    }
}
