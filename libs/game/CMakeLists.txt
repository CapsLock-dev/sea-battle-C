add_library(game)

target_link_libraries(game PUBLIC zxcurses)

target_sources(game
	PRIVATE	
		src/game.c
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/game/game.h
)
