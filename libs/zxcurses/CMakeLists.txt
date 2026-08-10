add_library(zxcurses)

#target_link_libraries(zxcurses PUBLIC datatype)

target_sources(zxcurses
	PRIVATE	
		src/screen.c
		src/terminal.c
        src/event_listener.c
        src/panel.c
        src/menu.c
        src/error.c
        src/color.c
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/zxcurses/screen.h
			include/zxcurses/terminal.h
			include/zxcurses/event_listener.h
            include/zxcurses/panel.h
            include/zxcurses/menu.h
            include/zxcurses/error.c
            include/zxcurses/color.c
)
