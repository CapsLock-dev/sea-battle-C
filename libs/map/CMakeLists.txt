option(MAP_TREE "Build map using 2-3-4 tree" OFF)
option(MAP_HASHTABLE "Build map using hash table tree" OFF)

if (MAP_TREE AND MAP_HASHTABLE)
    message(FATAL_ERROR "Choose only one implementation: MAP_TREE or MAP_HASHTABLE")
endif()

if (MAP_TREE) 
    SET(MAP_SRC src/map_tree.c)
    SET(BACKEND_LIB tree)
elseif(MAP_HASHTABLE)
    SET(MAP_SRC src/map_hashtable.c)
    SET(BACKEND_LIB hash_table)
else()
    message(FATAL_ERROR "Specify type of map. -DMAP_TREE=ON or -DMAP_HASHTABLE=ON")
endif()

add_library(map)

target_link_libraries(map PUBLIC datatype ${BACKEND_LIB})

target_sources(map
	PRIVATE	
        ${MAP_SRC}
        src/entry.c
	PUBLIC
		FILE_SET HEADERS
		BASE_DIRS include
		FILES
			include/map/map.h
			include/map/errors.h
            include/map/internal/entry.h
)
