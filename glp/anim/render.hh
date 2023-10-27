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

#include "indexbuffer.hh"

namespace render {

    enum class mode {
        points,
        line_strip,
        line_loop,
        lines,
        triangles,
        triangle_strip,
        triangle_fan,
    };

    static inline GLenum mode2glenum(mode m) {
        switch(m) {
            case mode::points: return GL_POINTS;
            case mode::line_strip: return GL_LINE_STRIP;
            case mode::line_loop: return GL_LINE_LOOP;
            case mode::lines: return GL_LINES;
            case mode::triangles: return GL_TRIANGLES;
            case mode::triangle_strip: return GL_TRIANGLE_STRIP;
            case mode::triangle_fan: return GL_TRIANGLE_FAN;
        }
        return 0;
    }

    inline void render(IndexBuffer& in_ibuf, mode m) {
        GLuint handle = in_ibuf.get_handle();
        GLuint indices = in_ibuf.get_count();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        glDrawElements(mode2glenum(m), indices, GL_UNSIGNED_INT, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    inline void render(GLuint vert_count, mode m) { glDrawArrays(mode2glenum(m), 0, vert_count); }

    inline void instanced(IndexBuffer& in_ibuf, mode m, GLuint count) {
        GLuint handle = in_ibuf.get_handle();
        GLuint indices = in_ibuf.get_count();

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);
        glDrawElementsInstanced(mode2glenum(m), indices, GL_UNSIGNED_INT, 0, count);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    inline void instanced(GLuint vert_count, mode m, GLuint count) { glDrawArraysInstanced(mode2glenum(m), 0, vert_count, count); }

}
