#include "game/game.h"
#include "zxcurses/screen.h"
#include "core/app.h"
#include "game/main_menu_scene.h"
#include "game/settings_scene.h"
#include <stdio.h>

#define DEFAULT_WIDTH 10
#define DEFAULT_HEIGHT 10
#define DEFAULT_SINGLE_SHIP 4
#define DEFAULT_DUO_SHIP 3
#define DEFAULT_TRIPLE_SHIP 2
#define DEFAULT_QUADRIPLE_SHIP 1

int run(int argc, char** argv) {
    (void)argc; (void)argv;
    bool running = true;
    init_view();
    termsize size = get_terminal_size();
    GameSettings settings = {
        .width = DEFAULT_WIDTH,
        .height = DEFAULT_HEIGHT,
        .single_ship_count = DEFAULT_SINGLE_SHIP,
        .duo_ship_count = DEFAULT_DUO_SHIP,
        .triple_ship_count = DEFAULT_TRIPLE_SHIP,
        .quadriple_ship_count = DEFAULT_QUADRIPLE_SHIP
    };   
    create_screen_buffer(size.width, size.height);
    TUIError ec = TUI_EC_Ok;
    char* err_msg = 0;
    while (running) {
        MainMenuOption main_menu_opt = start_main_menu_scene();
        switch (main_menu_opt) {
            case MAIN_MENU_OPTION_SINGLEPLAYER:
                start_game(settings);
                break;
            case MAIN_MENU_OPTION_MULTIPLAYER: 

                break;
            case MAIN_MENU_OPTION_SETTINGS:
                ec = start_settings_screen(&settings, &err_msg);
                if (ec != TUI_EC_Ok) running = false;
                break;
            case MAIN_MENU_OPTION_QUIT: 
                running = false;
                break;
        }
    }
    if (ec != TUI_EC_Ok) {
        printf("Error: %s ; code=%d", err_msg, ec);
    }
    return 0;
}
