#pragma once

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <cstring>

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

#include "utils.h"

class Shader {
    private:
        GLuint handle;
        std::map<std::string, GLuint> attributes;
        std::map<std::string, GLuint> uniforms;

        inline std::string read_file(const std::string& path) {
            std::ifstream file {path};
            std::stringstream content;
            content << file.rdbuf();
            file.close();
            return content.str();
        }

        inline GLuint compile(const GLenum shader, const std::string& code) {
            GLuint s = glCreateShader(shader);
            const char* source = code.c_str();

            glShaderSource(s, 1, &source, NULL);
            glCompileShader(s);

            int res = 0;
            glGetShaderiv(s, GL_COMPILE_STATUS, &res);
            if(!res) {
                char log[512];
                glGetShaderInfoLog(s, 512, NULL, log);
                glp_logv("%s shader compilation failed: %s", (shader == GL_VERTEX_SHADER ? "vertex" : "fragment"), log);
                glDeleteShader(s);
                return 0;
            }
            return s;
        }

        inline bool link_shaders(GLuint vertex, GLuint fragment) {
            glAttachShader(handle, vertex);
            glAttachShader(handle, fragment);
            glLinkProgram(handle);

            int res = 0;
            bool ret = true;
            glGetProgramiv(handle, GL_LINK_STATUS, &res);
            if(!res) {
                char log[512];
                glGetProgramInfoLog(handle, 512, NULL, log);
                glp_logv("shader linking failed: %s", log);
                ret = false;
            }

            glDeleteShader(vertex);
            glDeleteShader(fragment);
            return ret;
        }

        inline void populate_attributes() {
            int count = -1;
            int len;
            char name[128];
            int size;
            GLenum type;

            glUseProgram(handle);
            glGetProgramiv(handle, GL_ACTIVE_ATTRIBUTES, &count);

            for(int i=0; i<count; ++i) {
                memset(name, 0, sizeof(char)*128);
                glGetActiveAttrib(handle, (GLuint)i, 128, &len, &size, &type, name);
                int attrib = glGetAttribLocation(handle, name);
                if(attrib >= 0) attributes[name] = attrib;
            }

            glUseProgram(0);
        }

        inline void populate_uniforms() {
            int count = -1;
            int len;
            char name[128];
            int size;
            GLenum type;
            char tmp_name[256];

            glUseProgram(handle);
            glGetProgramiv(handle, GL_ACTIVE_UNIFORMS, &count);

            for(int i=0; i<count; ++i) {
                memset(name, 0, sizeof(char)*128);
                glGetActiveUniform(handle, (GLuint)i, 128, &len, &size, &type, name);
                int uniform = glGetUniformLocation(handle, name);
                if(uniform >= 0) {
                    std::string unif_name = name;
                    size_t found = unif_name.find('[');
                    if(found != std::string::npos) {
                        unif_name.erase(unif_name.begin() + found, unif_name.end());
                        GLuint unif_idx = 0;
                        while(true) {
                            memset(tmp_name, 0, sizeof(char)*256);
                            sprintf(tmp_name, "%s[%d]", unif_name.c_str(), unif_idx++);
                            int unif_loc = glGetUniformLocation(handle, tmp_name);
                            if(unif_loc < 0) break;
                            uniforms[tmp_name] = unif_loc;
                        }
                    }
                    uniforms[unif_name] = uniform;
                }
            }

            glUseProgram(0);
        }

        inline void make(const std::string& v, const std::string& f) {
            std::ifstream f1 {v.c_str()};
            std::ifstream f2 {f.c_str()};
            if(!f1 && !f2) return;
            f1.close(), f2.close();

            auto vert_s = read_file(v);
            auto vert = compile(GL_VERTEX_SHADER, vert_s);

            auto frag_s = read_file(f);
            auto frag = compile(GL_FRAGMENT_SHADER, frag_s);

            if(link_shaders(vert, frag)) {
                populate_attributes();
                populate_uniforms();
            }
        }

    public:
        inline Shader(const std::string& vertex, const std::string& fragment) {
            handle = glCreateProgram();
            make(vertex, fragment);
        }

        inline ~Shader() {
            glDeleteShader(handle);
        }

        inline void bind() { glUseProgram(handle); }
        inline void unbind() { glUseProgram(0); }

        inline GLuint get_handle() { return handle; }
        inline GLuint get_attribute(const std::string& name) {
            std::map<std::string, GLuint>::iterator it = attributes.find(name);
            if(it == attributes.end()) {
                glp_logv("bad attrib index: %s", name.c_str());
            }
            return it->second;
        }

        inline GLuint get_uniform(const std::string& name) {
            std::map<std::string, GLuint>::iterator it = uniforms.find(name);
            if(it == uniforms.end()) {
                glp_logv("bad uniform index: %s", name.c_str());
            }
            return it->second;
        }

        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
};
