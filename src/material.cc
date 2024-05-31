#include <material.hh>
#include <stb_image.h>

namespace glp {

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

Texture::Texture(const unsigned char* strliteral, const unsigned int len) {
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);

    int width, height, component;
    unsigned char* data = stbi_load_from_memory(strliteral, len, &width, &height, &component, 0);
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

Texture::~Texture() {
    glDeleteTextures(1, &id);
}

}
