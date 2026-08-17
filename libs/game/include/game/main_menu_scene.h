#ifndef CL_GAME_MAIN_MENU_SCENE_H
#define CL_GAME_MAIN_MENU_SCENE_H

typedef enum {
    MAIN_MENU_OPTION_SINGLEPLAYER,
    MAIN_MENU_OPTION_MULTIPLAYER,
    MAIN_MENU_OPTION_SETTINGS,
    MAIN_MENU_OPTION_QUIT
} MainMenuOption;

MainMenuOption start_main_menu_scene();

#endif
