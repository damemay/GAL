cmake_minimum_required(VERSION 3.5)

project(glp)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)

option(GLP_SAMPLE "Build sample" ON)
option(GLP_DEBUG "Build with debug options" OFF)

if(GLP_DEBUG)
    add_compile_options(-O1 -g -fsanitize=address -fno-omit-frame-pointer)
    add_compile_definitions(GLP_DEBUG)
else()
    add_compile_options(-O2)
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
    src/glp.cc
    src/sdl.cc
    src/gl.cc

    src/scene.cc
)

if(GLP_SAMPLE)
    set(EXEC_NAME sample)

    add_executable(${EXEC_NAME}
        main.cc
    )
    
    find_package(SDL2 REQUIRED)
    find_package(glm REQUIRED)
    find_package(Bullet REQUIRED)

    include_directories(
        glp/
        ext/glad/include/
        ext/stb/include/
        ext/tinygltf/
        ${BULLET_INCLUDE_DIRS}
    )
    
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
