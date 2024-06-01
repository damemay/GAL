#pragma once

#include <map>
#include <sdl.hh>
#include <glm/gtc/type_ptr.hpp>

namespace glp {
    namespace render {
        struct Material {
            GLuint shader;
            std::map<uint8_t, GLuint> textures;
            std::map<std::string, GLint> uniforms;

            glm::vec4 albedo {};
            float metallic {};
            float roughness {};

            int8_t albedo_id {-1};
            int8_t metallic_id {-1};
            int8_t roughness_id {-1};
            int8_t occlusion_id {-1};
            int8_t normal_id {-1};

            void generate_shader();
            void use() const { glUseProgram(shader); }
            void free() const { glUseProgram(0); }

            void set(GLint uniform, int value) const { glUniform1i(uniform, value); }
            void set(GLint uniform, float value) const { glUniform1f(uniform, value); }
            void set(GLint uniform, const glm::vec2& value) const { glUniform2fv(uniform, 1, &value[0]); }
            void set(GLint uniform, const glm::vec3& value) const { glUniform3fv(uniform, 1, &value[0]); }
            void set(GLint uniform, const glm::vec4& value) const { glUniform4fv(uniform, 1, &value[0]); }
            void set(GLint uniform, const glm::quat& value) const { glUniform4fv(uniform, 1, &value[0]); }
            void set(GLint uniform, const glm::mat4& value) const { glUniformMatrix4fv(uniform, 1, GL_FALSE, &value[0][0]); }
            void set(GLint uniform, const std::vector<glm::mat4>& value) const { glUniformMatrix4fv(uniform, value.size(), GL_FALSE, &value.data()[0][0][0]); }

            void set(const std::string& name, int value) const { glUniform1i(glGetUniformLocation(shader, name.c_str()), value); }
            void set(const std::string& name, float value) const { glUniform1f(glGetUniformLocation(shader, name.c_str()), value); }
            void set(const std::string& name, const glm::vec2& value) const { glUniform2fv(glGetUniformLocation(shader, name.c_str()), 1, &value[0]); }
            void set(const std::string& name, const glm::vec3& value) const { glUniform3fv(glGetUniformLocation(shader, name.c_str()), 1, &value[0]); }
            void set(const std::string& name, const glm::vec4& value) const { glUniform4fv(glGetUniformLocation(shader, name.c_str()), 1, &value[0]); }
            void set(const std::string& name, const glm::quat& value) const { glUniform4fv(glGetUniformLocation(shader, name.c_str()), 1, &value[0]); }
            void set(const std::string& name, const glm::mat4& value) const { glUniformMatrix4fv(glGetUniformLocation(shader, name.c_str()), 1, GL_FALSE, &value[0][0]); }
            void set(const std::string& name, const std::vector<glm::mat4>& value) const { glUniformMatrix4fv(glGetUniformLocation(shader, name.c_str()), value.size(), GL_FALSE, &value.data()[0][0][0]); }
        };
    }
}
