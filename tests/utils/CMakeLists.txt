add_library(test-utils)

target_link_libraries(test-utils PUBLIC datatype)

target_sources(test-utils
    PRIVATE 
        src/envelope_destructor.c 
        src/array_tools.c 
    PUBLIC
	    FILE_SET HEADERS
	    BASE_DIRS include
	    FILES
		    include/test-utils/array_tools.h
		    include/test-utils/envelope_destructor.h
)
