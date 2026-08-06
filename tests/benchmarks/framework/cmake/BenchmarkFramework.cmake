function(clb_add_benchmark target)
    add_custom_target(clb_run_${target}
        COMMAND $<TARGET_FILE:${target}>
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        DEPENDS ${target}
        USES_TERMINAL
    )
endfunction()
