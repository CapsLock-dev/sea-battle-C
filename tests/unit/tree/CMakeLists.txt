add_executable(tree_test)

target_link_libraries(tree_test PRIVATE cltest tree test-utils)

target_sources(tree_test
    PRIVATE 
		tree_test.c
)

clt_discover_tests(tree_test)
