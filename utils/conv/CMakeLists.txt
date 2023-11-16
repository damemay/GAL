cmake_minimum_required(VERSION 3.5)

project(glp-conv)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-g)
add_compile_definitions(USE_ASSIMP)

set(EXEC_NAME glp-conv)

include_directories(
    ../../glp
)

add_executable(${EXEC_NAME}
    ../../glp/external/glad.c
    ../../glp/external/stbi.c
    ../../src/utils.cc
    ../../src/sdl.cc
    ../../src/shader.cc
    ../../src/model.cc
    ../../src/anim.cc
    ../../src/material.cc
    main.cc
)

target_link_libraries(${EXEC_NAME}
    SDL2
    GL
    zstd
    assimp
    m
)
