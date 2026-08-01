add_library(core)

target_link_libraries(core PUBLIC zxcurses)

target_sources(core
	PRIVATE	
		src/app.c
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/core/app.h
)
