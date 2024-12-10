cmake_minimum_required(VERSION 3.11)

project(BIGWORK)


include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/task/classic ${PROJECT_SOURCE_DIR}/task ${PROJECT_SOURCE_DIR}/common)
link_directories(${PROJECT_SOURCE_DIR}/lib)

aux_source_directory(./common COMMON_LIST)
aux_source_directory(./task/classic CLASSIC_LIST)
aux_source_directory(./task TASK_LIST)

add_subdirectory(include/imgui)

add_executable(main src/main.cpp src/glad.c  ${TASK_LIST} ${CLASSIC_LIST} ${COMMON_LIST} ${IMGUI_LIST})
target_link_libraries(main glfw3 libassimpd imgui)

add_executable(offline_rendering src/main_offline.cpp src/glad.c ${TASK_LIST} ${CLASSIC_LIST} ${COMMON_LIST} ${IMGUI_LIST})
target_link_libraries(offline_rendering glfw3 libassimpd imgui)

add_executable(imgui_test src/imgui_test.cpp src/glad.c ${COMMON_LIST} ${IMGUI_LIST})
target_link_libraries(imgui_test glfw3 libassimpd imgui)

add_executable(test_main src/test_main.cpp)

add_custom_target(copy_assimp_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/bin/libassimp-5d.dll"
    "${PROJECT_SOURCE_DIR}/build"
    COMMENT "Copying libassimp-5d.dll to build directory"
)

# 添加目标：复制 source 文件夹到 build 文件夹
add_custom_target(copy_source_folder ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/source"  # 源文件夹
    "${PROJECT_BINARY_DIR}/source" # 目标文件夹
    COMMENT "Copying entire source folder (shaders, models, etc.) to build directory"
)
