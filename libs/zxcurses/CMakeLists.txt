add_library(zxcurses)

#target_link_libraries(zxcurses PUBLIC datatype)

target_sources(zxcurses
	PRIVATE	
		src/error.c
		src/screen.c
		src/terminal.c
        src/event_listener.c
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/zxcurses/error.h
			include/zxcurses/screen.h
			include/zxcurses/terminal.h
			include/zxcurses/event_listener.h
)
