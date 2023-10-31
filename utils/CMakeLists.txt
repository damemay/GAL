cmake_minimum_required(VERSION 3.5)

project(glp-util)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-g)

set(EXEC_NAME glp-util)

add_executable(${EXEC_NAME}
    ../glp/external/glad.c
    ../glp/external/stbi.c
    ../glp/utils.cc
    ../glp/sdl.cc
    ../glp/shader.cc
    ../glp/model.cc
    model.pb.cc
    main.cc
)

target_link_libraries(${EXEC_NAME}
    protobuf
    SDL2
    GL
    assimp
    m
)
