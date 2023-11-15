#pragma once

#include "shader.hh"

namespace glp {

struct Texture {
    GLuint id{0};
    std::string path{};

    Texture(const std::string& path);
    ~Texture();

    Texture(const Texture&) = delete;
    Texture& operator=(const Texture&) = delete;
};

enum class ShadingType {
    PHONG,
    PBR,
};

struct Material {
    std::vector<Texture*> textures;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    int8_t diffuse_id {-1};
    int8_t specular_id {-1};

    glm::vec3 albedo;
    float metallic;
    float roughness;

    int8_t metallic_id {-1};
    int8_t roughness_id {-1};
    int8_t ao_id {-1};

    int8_t normal_id {-1};

    Material() {}
    Material(const std::vector<Texture*>& texs) : textures{texs} {}
    Material(Texture* tex) { textures.push_back(tex); }
    ~Material() {}

    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
};

}
