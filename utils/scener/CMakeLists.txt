cmake_minimum_required(VERSION 3.5)

project(glp-scener)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-g)

set(EXEC_NAME glp-scener)

add_executable(${EXEC_NAME}
    ../../glp/external/glad.c
    ../../glp/external/stbi.c
    ../../glp/utils.cc
    ../../glp/sdl.cc
    ../../glp/shader.cc
    ../../glp/model.cc
    ../../glp/anim.cc
    ../../glp/fonts.cc
    ../../glp/obj/player.cc
    ../../glp/obj/placables.cc
    imgui/imgui.cpp
    imgui/imgui_demo.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_sdl2.cpp
    imgui/imgui_impl_opengl3.cpp
    main.cc
)

target_link_libraries(${EXEC_NAME}
    SDL2
    GL
    zstd
    assimp
    m
)
