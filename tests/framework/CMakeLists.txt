add_library(cltest)

target_sources(cltest
    PRIVATE 
        src/cltest.c 
    PUBLIC
	    FILE_SET HEADERS
	    BASE_DIRS include
	    FILES
		    include/cltest/cltest.h
)
