#pragma once

#include <limits>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "../utils/model.pb.h"

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

struct Bone {
    std::string name;
    glm::mat4 matrix;
};

template <typename T>
struct AnimationKey {
    T value;
    float time;
};

struct AnimationChannel {
    std::string node_name;
    std::vector<AnimationKey<glm::vec3>> position_keys;
    std::vector<AnimationKey<glm::quat>> rotation_keys;
    std::vector<AnimationKey<glm::vec3>> scale_keys;
};

struct Animation {
    std::string name;
    float duration;
    float ticks_per_second;
    std::vector<AnimationChannel> channels;
};

struct SkeletonNode {
    std::string name {""};
    int bone_index {-1};
    std::vector<SkeletonNode> children{};
    AnimationChannel keys;

    SkeletonNode* find(std::string name);
};

class Mesh {
    private:
        GLuint VAO, VBO, EBO;
        
    public:
        std::vector<Vertex>             vertices;
        std::vector<unsigned int>       indices;

        void render(Shader& shader);

        Mesh(std::vector<Vertex> vert, std::vector<unsigned int> idx);
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
};

class Model {
    private:
        std::vector<Mesh*> meshes {};
        std::vector<Texture*> textures {};
        std::vector<Animation> animations {};
        std::vector<Bone> bones {};
        std::vector<Bone> animation_bones {};

        SkeletonNode root_node;
        std::string directory;

        glm::mat4 animation_translate(const SkeletonNode& node, const Animation& animation);
        glm::mat4 animation_rotate(const SkeletonNode& node, const Animation& animation);
        glm::mat4 animation_scale(const SkeletonNode& node, const Animation& animation);
        void animation(const SkeletonNode& node, const Animation& anim);
        void animation_load(const Animation& animation);

        void assimp_load(const std::string& path);
        void assimp_node_process(aiNode* node, const aiScene* scene);
        int assimp_skeleton_import(aiNode* ai_node, SkeletonNode& node);
        void assimp_animation_load(aiAnimation* ai_anim);
        Mesh* assimp_mesh_process(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture*> assimp_textures_load(aiMaterial* mat, aiTextureType type);

        void protobuf_load(const std::string& path);

    public:
        void animate(uint32_t index);
        void animate(const std::string& name);
        void render(Shader& shader);

        void fill_protobuf(glp_util::Model* pb);

        Model(const std::string& path, bool assimp);
        ~Model();

};
