add_executable(hash_table_test)

target_link_libraries(hash_table_test PRIVATE cltest hash_table test-utils)

target_sources(hash_table_test
    PRIVATE 
		hash_table_test.c
)

clt_discover_tests(hash_table_test)
