cmake_minimum_required(VERSION 3.5)

project(glp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(GLP_SAMPLE "Build sample" ON)
option(GLP_DEBUG "Build with debug options" OFF)

if(CG_DEBUG)
    add_compile_options(-O1 -g -fsanitize=address -fno-omit-frame-pointer)
    add_compile_definitions(CG_DEBUG)
else()
    add_compile_definitions(-O2)
endif()

set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE INTERNAL "" FORCE)
add_subdirectory(ext/tinygltf)

add_library(glp STATIC
    ext/glad/src/glad.c
    ext/stb/src/stb.c
    ext/tinygltf.cc

    src/utils.cc
    src/sdl.cc
    src/shader.cc
    src/model.cc
    src/anim.cc
    src/fonts.cc
    src/material.cc
    src/player.cc
    src/renderable.cc
    src/collidable.cc
    src/scene.cc
)

if(GLP_SAMPLE)
    set(EXEC_NAME sample)

    add_executable(${EXEC_NAME}
        main.cc
    )

    include_directories(
        glp/
        ext/glad/include/
        ext/stb/include/
        ext/tinygltf/
        ${BULLET_INCLUDE_DIRS}
    )
    
    find_package(SDL2 REQUIRED)
    find_package(glm REQUIRED)
    find_package(Bullet REQUIRED)
    
    if(GLP_DEBUG)
        target_link_libraries(${EXEC_NAME}
            glp
            ${BULLET_LIBRARIES}
            glm::glm
            SDL2
            m
            -fsanitize=address
        )
    else()
        target_link_libraries(${EXEC_NAME}
            glp
            ${BULLET_LIBRARIES}
            glm::glm
            SDL2
            m
        )
    endif()
endif()
