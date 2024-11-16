cmake_minimum_required(VERSION 3.5)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(BIGWORK)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/task/classic ${PROJECT_SOURCE_DIR}/task ${PROJECT_SOURCE_DIR}/common)
link_directories(${PROJECT_SOURCE_DIR}/lib)

aux_source_directory(./common COMMON_LIST)
file(GLOB_RECURSE FLUID_LIST "fluid/*.cpp")
aux_source_directory(./task/classic CLASSIC_LIST)
aux_source_directory(./task TASK_LIST)

add_subdirectory(include/imgui)

add_executable(main src/main.cpp src/glad.c ${FLUID_LIST} ${TASK_LIST} ${CLASSIC_LIST} ${COMMON_LIST} ${IMGUI_LIST})
target_link_libraries(main glfw3 libassimpd imgui)

add_executable(offline_rendering src/main_offline.cpp src/glad.c ${FLUID_LIST} ${TASK_LIST} ${CLASSIC_LIST} ${COMMON_LIST} ${IMGUI_LIST})
target_link_libraries(offline_rendering glfw3 libassimpd imgui)

add_executable(imgui_test src/imgui_test.cpp src/glad.c ${COMMON_LIST} ${IMGUI_LIST})
target_link_libraries(imgui_test glfw3 libassimpd imgui)

add_executable(fluid_testbed src/fluid_testbed.cpp src/glad.c ${FLUID_LIST} ${COMMON_LIST})
target_compile_options(fluid_testbed PRIVATE -msse2)
target_link_libraries(fluid_testbed glfw3 libassimpd opengl32 glu32)

add_executable(fluid_test src/fluid_test.cpp src/glad.c ${FLUID_LIST} ${TASK_LIST} ${CLASSIC_LIST} ${COMMON_LIST} ${IMGUI_LIST})
target_link_libraries(fluid_test glfw3 libassimpd imgui)

add_custom_target(copy_assimp_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/bin/libassimp-5d.dll"
    "${PROJECT_SOURCE_DIR}/build"
    COMMENT "Copying libassimp-5d.dll to build directory"
)