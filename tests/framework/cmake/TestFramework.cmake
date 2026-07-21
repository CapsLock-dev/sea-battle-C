function(clt_discover_tests target)

    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND}
            -DEXECUTABLE_NAME=$<TARGET_FILE:${target}>
            -P "${CMAKE_SOURCE_DIR}/tests/framework/cmake/TestDiscovery.cmake"
        VERBATIM
    )

    set_property(DIRECTORY APPEND PROPERTY TEST_INCLUDE_FILES "${CMAKE_CURRENT_BINARY_DIR}/${target}.discovered_tests.cmake")

endfunction()
