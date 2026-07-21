add_library(hash_table)

target_link_libraries(hash_table PUBLIC datatype)

target_sources(hash_table
	PRIVATE	
		src/hash_table.c
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/hash_table/hash_table.h
			include/hash_table/errors.h
)
