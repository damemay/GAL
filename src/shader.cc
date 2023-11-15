#include "shader.hh"
#include "utils.hh"

GLuint Shader::compile(const GLenum shader, const std::string& code) {
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

bool Shader::link_shaders(GLuint vertex, GLuint fragment) {
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

void Shader::make(const std::string& v, const std::string& f) {
    auto vert_s = util::read_file(v);
    auto vert = compile(GL_VERTEX_SHADER, vert_s);

    auto frag_s = util::read_file(f);
    auto frag = compile(GL_FRAGMENT_SHADER, frag_s);

    link_shaders(vert, frag);
}

Shader::Shader(const std::string& vertex, const std::string& fragment, bool path) {
    handle = glCreateProgram();
    if(path) make(vertex, fragment);
    else {
        auto vert = compile(GL_VERTEX_SHADER, vertex);
        auto frag = compile(GL_FRAGMENT_SHADER, fragment);
        link_shaders(vert, frag);
    }
}
