add_library(tree)

target_link_libraries(tree PUBLIC datatype)

target_sources(tree
	PRIVATE	
		src/tree.c
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/tree/tree.h
			include/tree/errors.h
)
