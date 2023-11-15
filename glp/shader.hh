#pragma once

#include <map>
#include <string>
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
#include "external/glm/gtc/type_ptr.hpp"

#include "utils.hh"

namespace glp {

class Shader {
    private:
        GLuint handle;

        GLuint compile(const GLenum shader, const std::string& code);
        bool link_shaders(GLuint vertex, GLuint fragment);
        void make(const std::string& v, const std::string& f);
    public:
        Shader(const std::string& vertex, const std::string& fragment, bool from_path=true);

        inline ~Shader() {
            glDeleteShader(handle);
        }

        inline void bind() { glUseProgram(handle); }
        inline void unbind() { glUseProgram(0); }

        inline GLuint get() const { return handle; }

        inline void set(const std::string& name, int value) const { glUniform1i(glGetUniformLocation(handle, name.c_str()), value); }
        inline void set(const std::string& name, float value) const { glUniform1f(glGetUniformLocation(handle, name.c_str()), value); }
        inline void set(const std::string& name, const glm::vec2& value) const { glUniform2fv(glGetUniformLocation(handle, name.c_str()), 1, &value[0]); }
        inline void set(const std::string& name, const glm::vec3& value) const { glUniform3fv(glGetUniformLocation(handle, name.c_str()), 1, &value[0]); }
        inline void set(const std::string& name, const glm::vec4& value) const { glUniform4fv(glGetUniformLocation(handle, name.c_str()), 1, &value[0]); }
        inline void set(const std::string& name, const glm::quat& value) const { glUniform4fv(glGetUniformLocation(handle, name.c_str()), 1, &value[0]); }
        inline void set(const std::string& name, const glm::mat4& value) const { glUniformMatrix4fv(glGetUniformLocation(handle, name.c_str()), 1, GL_FALSE, &value[0][0]); }

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
};

}
