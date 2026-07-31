add_executable(map_test)

target_link_libraries(map_test PRIVATE cltest map test-utils)

target_sources(map_test
    PRIVATE 
		map_test.c
)

clt_discover_tests(map_test)
