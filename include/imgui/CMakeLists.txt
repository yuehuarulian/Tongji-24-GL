aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} IMGUI_SOURCES)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
