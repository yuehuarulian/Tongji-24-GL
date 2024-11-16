cmake_minimum_required(VERSION 3.5)

project(BIGWORK)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/task/classic ${PROJECT_SOURCE_DIR}/task ${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR}/openvdb_include)

link_directories(${PROJECT_SOURCE_DIR}/openvdb_bin)
link_directories(${PROJECT_SOURCE_DIR}/bin)
link_directories(${PROJECT_SOURCE_DIR}/lib)

set(DLL_FILES
    "${PROJECT_SOURCE_DIR}/bin/libassimp-5d.dll"
    "${PROJECT_SOURCE_DIR}/openvdb_bin/libblosc.dll"
    "${PROJECT_SOURCE_DIR}/openvdb_bin/liblz4.dll"
    "${PROJECT_SOURCE_DIR}/openvdb_bin/libopenvdb.dll"
    "${PROJECT_SOURCE_DIR}/openvdb_bin/libtbb12.dll"
    "${PROJECT_SOURCE_DIR}/openvdb_bin/libzlib1.dll"
    "${PROJECT_SOURCE_DIR}/openvdb_bin/libzstd.dll"
)

add_custom_target(copy_dlls ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/build"
)

# 对每个 DLL 文件添加一个自定义的命令来复制它们
foreach(DLL_FILE ${DLL_FILES})
    add_custom_command(TARGET copy_dlls
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLL_FILE} "${PROJECT_SOURCE_DIR}/build"
        COMMENT "Copying ${DLL_FILE} to build directory"
    )
endforeach()

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

add_executable(openvdb_test src/openvdb_test.cpp)
target_link_libraries(openvdb_test openvdb tbb12 zlib1 blosc)