cmake_minimum_required(VERSION 3.25)

project(SeaBattle C)

SET(EXEC_SRCS src/main.c)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(libs/datatype)
add_subdirectory(libs/hash_table)
add_subdirectory(libs/tree)
add_subdirectory(libs/map)
add_subdirectory(libs/core)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE ${EXEC_SRCS})
target_link_libraries(${PROJECT_NAME} PRIVATE core)

if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif(BUILD_TESTING)
