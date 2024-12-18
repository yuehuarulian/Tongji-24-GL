cmake_minimum_required(VERSION 3.11)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(BIGWORK)

if(MINGW)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wa,-mbig-obj")
endif()

# set(CMAKE_TOOLCHAIN_FILE "D:/ProgrammingSoftware/vcpkg/scripts/buildsystems/vcpkg.cmake")


include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/task/classic ${PROJECT_SOURCE_DIR}/task ${PROJECT_SOURCE_DIR}/common)
include_directories(${PROJECT_SOURCE_DIR}/openvdb_include)
include_directories(${PROJECT_SOURCE_DIR}/include/bullet)

link_directories(${PROJECT_SOURCE_DIR}/openvdb_bin)
link_directories(${PROJECT_SOURCE_DIR}/bin)
link_directories(${PROJECT_SOURCE_DIR}/lib)

set(DLL_FILES
    "${PROJECT_SOURCE_DIR}/bin/glfw3.dll"
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
file(GLOB_RECURSE FLUID_LIST "fluid/*.cpp")

add_subdirectory(include/imgui)

add_executable(main src/main.cpp src/glad.c  ${FLUID_LIST} ${TASK_LIST} ${CLASSIC_LIST} ${COMMON_LIST} ${IMGUI_LIST})
target_link_libraries(main glfw3 libassimpd imgui openvdb tbb12 zlib1 blosc
BulletDynamics   # Bullet 动力学
BulletCollision  # Bullet 碰撞检测
LinearMath       # Bullet 线性代数数学支持
BulletSoftBody   # Bullet 软体支持
)

# add_executable(offline_rendering src/main_offline.cpp src/glad.c ${FLUID_LIST}  ${TASK_LIST} ${CLASSIC_LIST} ${COMMON_LIST} ${IMGUI_LIST})
# target_link_libraries(offline_rendering glfw3 libassimpd imgui openvdb tbb12 zlib1 blosc
# BulletDynamics   # Bullet 动力学
# BulletCollision  # Bullet 碰撞检测
# LinearMath       # Bullet 线性代数数学支持
# BulletSoftBody   # Bullet 软体支持
# Bullet3Common    # Bullet 通用库
# )


# add_executable(fluid_testbed src/fluid_testbed.cpp src/glad.c ${FLUID_LIST} ${COMMON_LIST})
# target_compile_options(fluid_testbed PRIVATE -msse2)
# target_link_libraries(fluid_testbed glfw3 libassimpd opengl32 glu32)

# add_executable(fluid_test src/fluid_test.cpp src/glad.c ${FLUID_LIST} ${TASK_LIST} ${CLASSIC_LIST} ${COMMON_LIST} ${IMGUI_LIST})
# target_link_libraries(fluid_test glfw3 libassimpd imgui openvdb tbb12 zlib1 blosc)

# add_executable(imgui_test src/imgui_test.cpp src/glad.c ${COMMON_LIST} ${IMGUI_LIST})
# target_link_libraries(imgui_test glfw3 libassimpd imgui)

# add_executable(openvdb_test src/openvdb_test.cpp src/glad.c ${CLASSIC_LIST} ${COMMON_LIST} ${IMGUI_LIST})
# target_link_libraries(openvdb_test glfw3 libassimpd imgui openvdb tbb12 zlib1 blosc)

# add_executable(keyframe_test src/keyframe_test.cpp src/glad.c ${TASK_LIST} ${CLASSIC_LIST} ${COMMON_LIST} ${IMGUI_LIST})
# target_link_libraries(keyframe_test glfw3 libassimpd imgui openvdb tbb12 zlib1 blosc)

# add_executable(bullet_test src/bullet_test.cpp src/glad.c ${COMMON_LIST} ${IMGUI_LIST})
# target_link_libraries(bullet_test glfw3 libassimpd imgui openvdb  zlib1 blosc BulletDynamics BulletCollision LinearMath BulletSoftBody Bullet3Common Bullet3Geometry)

# add_executable(imgui_test src/imgui_test.cpp src/glad.c ${COMMON_LIST} ${IMGUI_LIST})
# target_link_libraries(imgui_test glfw3 libassimpd imgui)

# add_executable(test_main src/test_main.cpp)
