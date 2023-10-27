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
#include "external/glm/gtc/quaternion.hpp"
#include "external/glm/mat4x4.hpp"

template <typename T>
class Uniform {
    public:
        Uniform() = delete;
        Uniform(const Uniform&) = delete;
        Uniform& operator=(const Uniform&) = delete;
        ~Uniform() = delete;

        static void set(GLuint slot, T* arr, GLuint len);

        inline static void set(GLuint slot, const T& value) {
            set(slot, (T*)&value, 1);
        }

        inline static void set(GLuint slot, std::vector<T>& arr) {
            set(slot, &arr[0], (GLuint)arr.size());
        }
};
