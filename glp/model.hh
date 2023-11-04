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
        std::vector<Texture*>           textures;

        void render(Shader* shader);

        Mesh(std::vector<Vertex> vert, std::vector<unsigned int> idx, std::vector<Texture*> tex);
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
};

class Model {
    private:
        std::vector<Mesh*> meshes {};
        std::vector<BoneInfo> bones {};
        std::vector<Texture*> textures {};

        Shader* shader;
        std::string directory;

#ifdef USE_ASSIMP
        void assimp_load(const std::string& path);
        void assimp_node_process(aiNode* node, const aiScene* scene);
        Mesh* assimp_mesh_process(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture*> assimp_textures_load(aiMaterial* mat, aiTextureType type);
#endif
        Texture* texture_load(const std::string& path);
        void deserialize_data(std::stringstream& s);

    public:
        void render();

        inline const std::vector<BoneInfo>& get_bone_info() const { return bones; }
        inline const Shader* get_shader() const { return shader; }

        std::stringstream serialize_data();

#ifdef USE_ASSIMP
        Model(const std::string& path, bool assimp, Shader* shader);
#endif
        Model(const std::string& path, Shader* shader);
        ~Model();

};
