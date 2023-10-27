#pragma once

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_USE_CPP14
#include "external/tiny_gltf.h"

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

struct vao_range {
    GLsizei begin;
    GLsizei count;
};

class Model {
    private:
        std::vector<GLuint> buffer_objects;
        std::vector<vao_range> mesh_vaos;
        std::vector<GLuint> vaos;

        bool load(std::string path);

    public:
        Model(std::string path);
        ~Model();
};
