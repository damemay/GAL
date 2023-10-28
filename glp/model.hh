#pragma once

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_USE_CPP14
#include "external/tiny_gltf.h"

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

struct IndexData {
    size_t count{0};
    size_t offset{0};
    int type{-1};
};

struct Mesh {
    bool has_indices{false};
    IndexData indices;
    uint8_t vao_index{0};
    size_t count;
    GLenum mode{0};
};

struct Node {
    Node* parent;

    Mesh* mesh {nullptr};

    glm::vec3 translation       {0.0f, 0.0f, 0.0f};
    glm::quat rotation          {0.0f, 0.0f, 0.0f, 1.0f};
    glm::vec3 scale             {1.0f, 1.0f, 1.0f};
    glm::mat4 matrix            {1.0f};

    std::vector<Node*> children;

    inline ~Node() { delete mesh; for(auto& child: children) delete child; }
};

class Model {
    private:
        std::vector<GLuint> VBO;
        std::vector<GLuint> VAO;
        std::vector<Node*> nodes;
        std::vector<Texture> textures;
        Shader* shader;

        bool attribute_mesh(const std::string& name, const GLuint index, Mesh*& new_mesh, const tinygltf::Model& model, const tinygltf::Primitive& primitive);
        Mesh* load_mesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh);
        void load_node(Node* parent, const tinygltf::Node& node, const tinygltf::Model& model);
        void gen_buffers(const tinygltf::Model& model);
        bool load(const std::string& path);
        void render_node(Node* node, glm::mat4 transform, const glm::mat4& mat, const std::string& uniform);

    public:
        Model(const std::string& path, Shader& shader);
        void render(const glm::mat4& mat, const std::string& uniform);
        ~Model();
};
