execute_process(COMMAND ${EXECUTABLE_NAME} "--list"
    OUTPUT_VARIABLE TEST_LIST
)

string(REGEX REPLACE "\n" ";" TEST_LIST_OUT "${TEST_LIST}")

file(WRITE "${EXECUTABLE_NAME}.discovered_tests.cmake" "")

get_filename_component(FILE_NAME ${EXECUTABLE_NAME} NAME)

foreach(test_name ${TEST_LIST_OUT})
	file(APPEND "${EXECUTABLE_NAME}.discovered_tests.cmake" "add_test(${FILE_NAME}_${test_name} ${EXECUTABLE_NAME} ${test_name})\n")
endforeach()
