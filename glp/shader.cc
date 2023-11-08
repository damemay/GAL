#include "shader.hh"
#include "utils.hh"
#include "external/stb_image.h"

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

Texture::Texture(const std::string& path_) : path{path_} {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    int width, height, component;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &component, 0);
    glTexImage2D(GL_TEXTURE_2D, 0,
            (component == 3 ? GL_RGB : GL_RGBA), width, height, 0,
            (component == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, data);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::Texture(GLsizei width, GLsizei height, uint8_t format, void* pixels) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D, 0,
            (format == 3 ? GL_RGB : GL_RGBA), width, height, 0,
            (format == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, pixels);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() {
    glDeleteTextures(1, &id);
}
