# 设置最低的 CMake 版本要求
cmake_minimum_required(VERSION 3.11)

# 设置 C++ 标准为 C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置项目名称
project(bigwork)

# 设置包含的目录路径（包括头文件）
set(INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/include    # 项目头文件目录
    ${PROJECT_SOURCE_DIR}/common     # 公共函数目录
    ${PROJECT_SOURCE_DIR}/common/Basic
    ${PROJECT_SOURCE_DIR}/common/RTX
    ${PROJECT_SOURCE_DIR}/task       # 任务目录
    ${PROJECT_SOURCE_DIR}/task/classic # 经典任务目录
)

# 设置静态库文件路径（存放预编译的库）
set(LIB_DIR ${PROJECT_SOURCE_DIR}/lib)

# 设置 GLAD 源文件路径（OpenGL 函数加载器）
set(GLAD_SRC ${PROJECT_SOURCE_DIR}/src/glad.c)

# 设置公共源文件目录
set(COMMON_SRC ${PROJECT_SOURCE_DIR}/common)

# 设置 Basic 模块的源文件路径
set(COMMON_BASIC_SRC ${PROJECT_SOURCE_DIR}/common/Basic)

# 设置 RTX 模块的源文件路径
set(COMMON_RTX_SRC ${PROJECT_SOURCE_DIR}/common/RTX)

# 将静态库文件路径添加到链接目录
link_directories(${LIB_DIR})

# 包含目录（告诉编译器在哪里找到头文件）
include_directories(${INCLUDE_DIRS})

# 自动收集所有源文件
# 这里会将目录下所有的 `.cpp` 文件添加到源文件列表中
aux_source_directory(${COMMON_SRC} COMMON_LIST)
aux_source_directory(${COMMON_BASIC_SRC} COMMON_BASIC_LIST)

# 设置输出目录（编译后的可执行文件存放位置）
set(OUTPUT_DIR "${PROJECT_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# 添加可执行文件：main
# 这里将源文件添加到可执行文件的构建中
add_executable(main src/main.cpp ${GLAD_SRC} ${COMMON_LIST})
target_link_libraries(main glfw3 ${LIB_DIR}/libassimp-5d.dll)

# 添加可执行文件：rayTracing
add_executable(rayTracing src/workbench.cpp)

# 添加可执行文件：01_image
add_executable(01_image src/01_Image.cpp ${COMMON_BASIC_LIST})

# 自定义目标：复制 DLL 文件到输出目录
# 确保在构建时将依赖的 DLL 文件（如 libassimp-5d.dll）复制到输出目录
add_custom_target(copy_assimp_dll ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${LIB_DIR}/libassimp-5d.dll"
    "${OUTPUT_DIR}/libassimp-5d.dll"
    COMMENT "Copying libassimp-5d.dll to output directory"
)

# 打印调试信息（可选，便于调试路径）
message(STATUS "Project source dir: ${PROJECT_SOURCE_DIR}")
message(STATUS "Include dirs: ${INCLUDE_DIRS}")
message(STATUS "Library dir: ${LIB_DIR}")
message(STATUS "Output dir: ${OUTPUT_DIR}")