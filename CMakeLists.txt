cmake_minimum_required(VERSION 3.5)

project(bigworlk)

include_directories(./include ./common ./task)

link_directories(./lib)
# find_library(assimp-5d TOOLS_SHARED ./bin)

aux_source_directory(./common COMMON_LIST)

add_executable(main src/main.cpp src/glad.c task/sphere.cpp ${COMMON_LIST})
target_link_libraries(main glfw3 libassimpd)

add_custom_target(copy_assimp_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/bin/libassimp-5d.dll"
    "${PROJECT_SOURCE_DIR}/build"
    COMMENT "Copying libassimp-5d.dll to build directory"
)
