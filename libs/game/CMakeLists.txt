add_library(game)

target_link_libraries(game PUBLIC zxcurses)

target_sources(game
	PRIVATE	
		src/game.c
        src/main_menu_scene.c
        src/settings_scene.c
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/game/game.h
            include/game/main_menu_scene.h
            include/game/settings_scene.h
)
