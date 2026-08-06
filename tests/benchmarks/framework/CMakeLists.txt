add_library(clbench)

target_sources(clbench
    PRIVATE 
        src/clbench.c 
    PUBLIC
	    FILE_SET HEADERS
	    BASE_DIRS include
	    FILES
		    include/clbench/clbench.h
)
