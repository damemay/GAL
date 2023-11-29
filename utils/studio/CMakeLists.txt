cmake_minimum_required(VERSION 3.5)

project(glp-studio)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-g)
add_compile_definitions(USE_ASSIMP)

set(BULLET_NAME glp-studio-bullet)

find_package(Bullet REQUIRED)

include_directories(
    ../../glp
    ${BULLET_INCLUDE_DIRS}
)

add_library(glp STATIC
    ../../glp/external/glad.c
    ../../glp/external/stbi.c
    ../../src/utils.cc
    ../../src/sdl.cc
    ../../src/shader.cc
    ../../src/model.cc
    ../../src/anim.cc
    ../../src/fonts.cc
    ../../src/material.cc
    ../../src/player.cc
    ../../src/renderable.cc
    ../../src/collidable.cc
    ../../src/scene.cc
)

add_library(imgui STATIC
    ../../glp/external/imgui/imgui.cpp
    ../../glp/external/imgui/imgui_demo.cpp
    ../../glp/external/imgui/imgui_draw.cpp
    ../../glp/external/imgui/imgui_tables.cpp
    ../../glp/external/imgui/imgui_widgets.cpp
    ../../glp/external/imgui/imgui_impl_sdl2.cpp
    ../../glp/external/imgui/imgui_impl_opengl3.cpp
)

add_executable(${BULLET_NAME}
    bullet.cc
)

target_link_libraries(${BULLET_NAME}
    glp
    ${BULLET_LIBRARIES}
    imgui
    SDL2
    GL
    zstd
    assimp
    m
)
