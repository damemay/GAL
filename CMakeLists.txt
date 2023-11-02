cmake_minimum_required(VERSION 3.5)

project(glp)
find_package(SDL2 REQUIRED)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 20)
add_compile_options(-g)

set(EXEC_NAME game)

add_executable(${EXEC_NAME}
    glp/external/glad.c
    glp/external/stbi.c
    glp/utils.cc
    glp/sdl.cc
    glp/shader.cc
    glp/model.cc
    glp/anim.cc
    #utils/model.pb.cc
    main.cc
)

target_link_libraries(${EXEC_NAME}
    #protobuf
    SDL2
    GL
    assimp
    m
)
