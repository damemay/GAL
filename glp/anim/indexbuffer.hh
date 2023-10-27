#pragma once

#include <vector>

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

class IndexBuffer {
    protected:
        GLuint handle;
        GLuint count;

    public:
        inline IndexBuffer() {
            glGenBuffers(1, &handle);
            count = 0;
        }

        inline ~IndexBuffer() {
            glDeleteBuffers(1, &handle);
        }

        inline void set(GLuint* arr, GLuint len) {
            count = len;
            GLuint size = sizeof(GLuint);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, size*count, arr, GL_STATIC_DRAW);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }

        inline void set(std::vector<GLuint>& in) {
            set(&in[0], (GLuint)in.size());
        }

        inline GLuint get_count() { return count; }
        inline GLuint get_handle() { return handle; }

        IndexBuffer(const IndexBuffer& i) = delete;
        IndexBuffer& operator=(const IndexBuffer& i) = delete;
};
