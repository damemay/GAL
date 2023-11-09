cmake_minimum_required(VERSION 3.5)

project(glp-conv)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-g)
add_compile_definitions(USE_ASSIMP)

set(EXEC_NAME glp-conv)

add_executable(${EXEC_NAME}
    ../../glp/external/glad.c
    ../../glp/external/stbi.c
    ../../glp/utils.cc
    ../../glp/sdl.cc
    ../../glp/shader.cc
    ../../glp/model.cc
    ../../glp/anim.cc
    main.cc
)

target_link_libraries(${EXEC_NAME}
    SDL2
    GL
    zstd
    assimp
    m
)
