#pragma once

#include <limits>
#include <vector>

#ifdef USE_ASSIMP
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#endif

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
#include "external/glm/gtc/matrix_transform.hpp"
#include "external/glm/gtc/type_ptr.hpp"

#include "shader.hh"
#include "material.hh"

namespace glp {

constexpr GLuint POSITION_ATTRIBUTE_INDEX       = 0;
constexpr GLuint NORMAL_ATTRIBUTE_INDEX         = 1;
constexpr GLuint TEXCOORD0_ATTRIBUTE_INDEX      = 2;
constexpr GLuint JOINTS_ATTRIBUTE_INDEX         = 3;
constexpr GLuint WEIGHTS_ATTRIBUTE_INDEX        = 4;

constexpr GLuint MAX_BONE_INFLUENCE             = 4;
constexpr GLuint MAX_BONES                      = 100;

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    float bone_index[MAX_BONE_INFLUENCE];
    float weights[MAX_BONE_INFLUENCE];
};

struct BoneInfo {
    std::string name;
    glm::mat4 offset;
};

class Mesh {
    private:
        GLuint VAO, VBO, EBO;
        
    public:
        std::vector<Vertex>             vertices;
        std::vector<unsigned int>       indices;
        Material*                       material;

        void render(Shader* shader, ShadingType type);

        Mesh(std::vector<Vertex> vert, std::vector<unsigned int> idx, Material* mat);
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
};

class Model {
    private:
        std::vector<Mesh*> meshes {};
        std::vector<BoneInfo> bones {};
        std::vector<Texture*> textures {};

        ShadingType shading {ShadingType::PBR};

        Shader* shader;
        std::string directory;

#ifdef USE_ASSIMP
        void assimp_load(const std::string& path);
        void assimp_node_process(aiNode* node, const aiScene* scene);
        Mesh* assimp_mesh_process(aiMesh* mesh, const aiScene* scene);
        Texture* assimp_textures_load(aiMaterial* mat, aiTextureType type);
#endif
        Texture* texture_load(const std::string& path);
        void deserialize_data(std::stringstream& s);

    public:
        void render();

        inline const std::vector<BoneInfo>& get_bone_info() const { return bones; }

        inline Shader* get_shader() { return shader; }
        inline void set_shader(Shader* s) { shader = s; }
        inline void set_shading_type(ShadingType s) { shading = s; }

        inline std::vector<Mesh*> get_meshes() { return meshes; }
        inline std::vector<Texture*> get_textures() { return textures; }

        glm::vec3 calculate_bounding_box();
        std::vector<glm::vec3> calculate_bounding_boxes();

        void load(const std::string& path);

        std::stringstream serialize_data();

        Model() {};
        Model(const std::string& path);
        ~Model();

};

}
