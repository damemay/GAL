cmake_minimum_required(VERSION 3.5)

project(gal)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

option(GAL_SAMPLE "Build sample" ON)
option(GAL_DEBUG "Build with debug options" OFF)

if(GAL_DEBUG)
    add_compile_options(-O1 -g -fsanitize=address -fno-omit-frame-pointer)
    add_compile_definitions(GAL_DEBUG)
else()
    add_compile_options(-O2)
endif()

set(TINYGLTF_HEADER_ONLY ON CACHE INTERNAL "" FORCE)
set(TINYGLTF_INSTALL OFF CACHE INTERNAL "" FORCE)
set(TINYGLTF_BUILD_LOADER_EXAMPLE OFF CACHE INTERNAL "" FORCE)
add_subdirectory(ext/tinygltf)

add_library(gal STATIC
    ext/glad/src/glad.c
    ext/stb/src/stb.c
    ext/tinygltf.cc

    src/util.cc
    src/gal.cc
    src/sdl.cc
    src/gl.cc
    src/gltf.cc

    src/material.cc
    src/scene.cc
    src/camera.cc
    src/controller.cc
    src/model.cc
    src/shader.cc
    src/shader_generator.cc
    src/environment.cc
)

if(GAL_SAMPLE)
    set(EXEC_NAME sample)

    add_executable(${EXEC_NAME}
        main.cc
    )
    
    find_package(SDL2 REQUIRED)
    find_package(glm REQUIRED)
    find_package(Bullet REQUIRED)

    include_directories(
        gal/
        ext/glad/include/
        ext/stb/include/
        ext/tinygltf/
        ${BULLET_INCLUDE_DIRS}
    )
    
    if(GAL_DEBUG)
        target_link_libraries(${EXEC_NAME}
            gal
            ${BULLET_LIBRARIES}
            glm::glm
            SDL2
            m
            -fsanitize=address
        )
    else()
        target_link_libraries(${EXEC_NAME}
            gal
            ${BULLET_LIBRARIES}
            glm::glm
            SDL2
            m
        )
    endif()
endif()
