#include "attribute.hh"
#include "uniform.hh"

#define UNIFORM_IMPL(gl_func, tType, dType) \
    template<> void Uniform<tType>::set(GLuint slot, tType* data, GLuint len) { \
        gl_func(slot, (GLsizei)len, (dType*)&data[0]); \
    }

UNIFORM_IMPL(glUniform1iv, int, int);
UNIFORM_IMPL(glUniform1fv, float, float);
UNIFORM_IMPL(glUniform2fv, glm::vec2, float);
UNIFORM_IMPL(glUniform3fv, glm::vec3, float);
UNIFORM_IMPL(glUniform4fv, glm::vec4, float);
UNIFORM_IMPL(glUniform4fv, glm::quat, float);

template<>
void Uniform<glm::mat4>::set(GLuint slot, glm::mat4* data, GLuint len) {
    glUniformMatrix4fv(slot, (GLsizei)len, false, (float*)&data[0]);
}
