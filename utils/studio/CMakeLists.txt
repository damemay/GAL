cmake_minimum_required(VERSION 3.5)

project(glp-conv)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-g)
add_compile_definitions(USE_ASSIMP)

set(EXEC_NAME glp-studio)

include_directories(
    ../../glp
)

add_executable(${EXEC_NAME}
    main.cc
    ../../glp/external/imgui/imgui.cpp
    ../../glp/external/imgui/imgui_demo.cpp
    ../../glp/external/imgui/imgui_draw.cpp
    ../../glp/external/imgui/imgui_tables.cpp
    ../../glp/external/imgui/imgui_widgets.cpp
    ../../glp/external/imgui/imgui_impl_sdl2.cpp
    ../../glp/external/imgui/imgui_impl_opengl3.cpp
)

target_link_libraries(${EXEC_NAME}
    ${CMAKE_SOURCE_DIR}/../../build/libglp.a
    SDL2
    GL
    zstd
    assimp
    m
)
