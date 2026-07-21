add_executable(datatype_test)

target_link_libraries(datatype_test PRIVATE cltest datatype)

target_sources(datatype_test
    PRIVATE 
		datatype_test.c
)

clt_discover_tests(datatype_test)
