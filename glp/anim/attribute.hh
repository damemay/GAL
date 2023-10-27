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

#include "external/glm/glm.hpp"

template<typename T>
class Attribute {
    protected:
        GLuint handle;
        GLuint count;

    private:
        void set_attrib_ptr(GLuint slot);

    public:
        inline Attribute() {
            glGenBuffers(1, &handle);
            count = 0;
        }

        inline ~Attribute() {
            glDeleteBuffers(1, &handle);
        }

        inline void set(T* input_arr, GLuint array_len) {
            count = array_len;
            GLuint size = sizeof(T);

            glBindBuffer(GL_ARRAY_BUFFER, handle);
            glBufferData(GL_ARRAY_BUFFER, size*count, input_arr, GL_STREAM_DRAW);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        inline void set(std::vector<T>& input) { set(&input[0], (GLuint)input.size()); }

        void bind_to(GLuint slot) {
            glBindBuffer(GL_ARRAY_BUFFER, handle);
            glEnableVertexAttribArray(slot);
            set_attrib_ptr(slot);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        void unbind_from(GLuint slot) {
            glBindBuffer(GL_ARRAY_BUFFER, handle);
            glDisableVertexAttribArray(slot);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        inline GLuint get_count() { return count; }
        inline GLuint get_handle() { return handle; }
        
        Attribute(const Attribute& a) = delete;
        Attribute& operator=(const Attribute& a) = delete;
};

template<>
inline void Attribute<int>::set_attrib_ptr(GLuint slot) { glVertexAttribIPointer(slot, 1, GL_INT, 0, (void*)0); }

template<>
inline void Attribute<float>::set_attrib_ptr(GLuint slot) { glVertexAttribPointer(slot, 1, GL_FLOAT, GL_FALSE, 0, 0); }

template<>
inline void Attribute<glm::vec2>::set_attrib_ptr(GLuint slot) { glVertexAttribPointer(slot, 2, GL_FLOAT, GL_FALSE, 0, 0); }

template<>
inline void Attribute<glm::vec3>::set_attrib_ptr(GLuint slot) { glVertexAttribPointer(slot, 3, GL_FLOAT, GL_FALSE, 0, 0); }

template<>
inline void Attribute<glm::vec4>::set_attrib_ptr(GLuint slot) { glVertexAttribPointer(slot, 4, GL_FLOAT, GL_FALSE, 0, 0); }

template<>
inline void Attribute<glm::ivec4>::set_attrib_ptr(GLuint slot) { glVertexAttribIPointer(slot, 4, GL_INT, 0, (void*)0); }
