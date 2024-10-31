cmake_minimum_required(VERSION 3.5)

project(bigworlk)

include_directories(./include ./common ./task)

link_directories(./lib)
# find_library(assimp-5d TOOLS_SHARED ./bin)

aux_source_directory(./common COMMON_LIST)
aux_source_directory(./task TASK_LIST)

add_executable(main src/main.cpp src/glad.c ${COMMON_LIST} ${TASK_LIST})
target_link_libraries(main glfw3 libassimpd)

add_executable(offline_rendering src/main_offline.cpp src/glad.c ${COMMON_LIST} ${TASK_LIST})
target_link_libraries(offline_rendering glfw3 libassimpd)

add_custom_target(copy_assimp_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/bin/libassimp-5d.dll"
    "${PROJECT_SOURCE_DIR}/build"
    COMMENT "Copying libassimp-5d.dll to build directory"
)
