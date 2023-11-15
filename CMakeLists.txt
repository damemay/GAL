cmake_minimum_required(VERSION 3.5)

option(VITA "Build for PS Vita" OFF)

if(VITA)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    
    project(glp)
      include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)
    
    set(VITA_APP_NAME "GLP")
    set(VITA_TITLEID  "GLPP07777")
    set(VITA_VERSION  "01.00")
    
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c17")
    set(CMAKE_CXX_STANDARD 20)
    
    set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d ATTRIBUTE2=12")
    
    include_directories(
      glp/
    )

    add_library(glp STATIC
        glp/external/stbi.c
        glp/utils.cc
        glp/sdl.cc
        glp/shader.cc
        glp/model.cc
        glp/anim.cc
        glp/fonts.cc
        glp/obj/player.cc
        glp/obj/placables.cc
    )
    
    add_executable(${PROJECT_NAME}
        main.cc
    )
    
    target_link_libraries(${PROJECT_NAME}
      glp
      SDL2
      SDL2main
      vitaGL
      mathneon
      freetype
      vorbisfile
      vorbis
      mikmod
      modplug
      ogg
      mpg123
      FLAC
      webp
      jpeg
      zstd
      png
      pthread
      vitashark
      SceShaccCgExt
      SceDisplay_stub
      SceCtrl_stub
      SceAudio_stub
      SceAudioIn_stub
      SceSysmodule_stub
      SceMotion_stub
      SceGxm_stub
      SceCommonDialog_stub
      SceTouch_stub
      SceHid_stub
      SceLibKernel_stub
      SceShaccCg_stub
      SceIme_stub
      SceAppMgr_stub
      SceKernelDmacMgr_stub
      taihen_stub
      m
      z
      c
    )
    
    vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME})
    vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
      VERSION ${VITA_VERSION}
      NAME ${VITA_APP_NAME}
      FILE sce_sys/icon0.png sce_sys/icon0.png
      FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
      FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
      FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
      FILE res res
    )
else()
    project(glp)
    find_package(SDL2 REQUIRED)
    find_package(Bullet REQUIRED)
    set(CMAKE_CXX_COMPILER g++)
    set(CMAKE_CXX_STANDARD 20)
    add_compile_options(-g)
    
    set(EXEC_NAME game)
    
    option(USE_ASSIMP "Link with assimp" OFF)
    if(USE_ASSIMP)
        add_compile_definitions(USE_ASSIMP)
    endif()

    include_directories(
      glp/
      ${BULLET_INCLUDE_DIRS}
    )

    add_library(glp STATIC
        glp/external/glad.c
        glp/external/stbi.c
        src/utils.cc
        src/sdl.cc
        src/shader.cc
        src/model.cc
        src/anim.cc
        src/fonts.cc
        src/material.cc
        src/player.cc
    )

    add_executable(${EXEC_NAME}
        main.cc
    )
    
    if(USE_ASSIMP)
    target_link_libraries(${EXEC_NAME}
        glp
        SDL2
        assimp
        GL
        zstd
        m
    )
    else()
    target_link_libraries(${EXEC_NAME}
        glp
        ${BULLET_LIBRARIES}
        SDL2
        GL
        zstd
        m
    )
    endif()
endif()
