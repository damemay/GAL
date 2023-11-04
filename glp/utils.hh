#pragma once

#include <string>
#include <zstd.h>

#ifndef __vita__
    #include "external/glad/glad.h"
    #ifdef __APPLE__
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>
    #endif
#else
    #include <vitaGL.h>
#endif

#define glp_diev(msg, ...) printf("glp: " msg "\n", __VA_ARGS__), exit(EXIT_FAILURE)
#define glp_die(msg) printf("glp: " msg "\n")
#define glp_logv(msg, ...) printf("glp: " msg "\n", __VA_ARGS__)
#define glp_log(msg) printf("glp: " msg "\n")

namespace util {

std::string read_file(const std::string& path);
std::string compress(const std::string& data, int compress_level);
std::string decompress(const std::string& data);

inline bool glerr() {
    GLenum err;
    while((err=glGetError())!=GL_NO_ERROR) {
        glp_logv("GL %d", err);
        return true;
    }
    return false;
}

}
