#pragma once

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

#include "external/stb_image.h"

class Texture {
    protected:
        GLuint width {0};
        GLuint height {0};
        GLuint channels {0};
        GLuint handle;

    public:
        inline Texture(const char* path) {
            glGenTextures(1, &handle);
            glBindTexture(GL_TEXTURE_2D, handle);

            int w,h,c;
            unsigned char* data = stbi_load(path, &w,&h,&c, 4);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w,h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            glBindTexture(GL_TEXTURE_2D, 0);

            width = w, height = h, channels = c;
        }

        inline ~Texture() {
            glDeleteTextures(1, &handle);
        }

        inline void set(GLuint uniform_idx, GLuint texture_idx) {
            glActiveTexture(GL_TEXTURE0 + texture_idx);
            glBindTexture(GL_TEXTURE_2D, handle);
            glUniform1i(uniform_idx, texture_idx);
        }

        inline void unset(GLuint texture_idx) {
            glActiveTexture(GL_TEXTURE0 + texture_idx);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE0);
        }

        inline GLuint get_handle() { return handle; }

        Texture(const Texture& t) = delete;
        Texture& operator=(const Texture& t) = delete;
};
