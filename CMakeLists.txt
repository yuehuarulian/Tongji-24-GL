cmake_minimum_required(VERSION 3.11)

project(bigworlk)

# 定义路径变量
set(INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/common
    ${PROJECT_SOURCE_DIR}/task
    ${PROJECT_SOURCE_DIR}/task/classic
)
set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)
set(GLAD_SRC ${PROJECT_SOURCE_DIR}/src/glad.c)
set(COMMON_SRC ${PROJECT_SOURCE_DIR}/common)

link_directories(${LIB_DIR})

# 包含目录
include_directories(${INCLUDE_DIRS})

# 源文件列表
aux_source_directory(${COMMON_SRC} COMMON_LIST)

# 设置输出目录
set(OUTPUT_DIR "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# 可执行文件
add_executable(main src/main.cpp ${GLAD_SRC} ${COMMON_LIST})
target_link_libraries(main glfw3 ${LIB_DIR}/libassimp-5d.dll)

# 自定义目标以复制 DLL
add_custom_target(copy_assimp_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LIB_DIR}/libassimp-5d.dll"
    "${OUTPUT_DIR}/libassimp-5d.dll"
    COMMENT "Copying libassimp-5d.dll to output directory"
)
