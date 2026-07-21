add_library(datatype)

target_sources(datatype
	PRIVATE	
		src/datatype.c
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/datatype/datatype.h
)
