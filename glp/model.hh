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

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

class Mesh {
    private:
        GLuint VAO, VBO, EBO;
        
    public:
        std::vector<Vertex>             vertices;
        std::vector<unsigned int>       indices;
        std::vector<Texture*>           textures;

        void render(Shader& shader);

        Mesh(std::vector<Vertex> vert, std::vector<unsigned int> idx, std::vector<Texture*> tex);
        ~Mesh();

        Mesh(const Mesh&) = delete;
        Mesh& operator=(const Mesh&) = delete;
};

class Model {
    private:
        std::vector<Mesh*> meshes;
        std::string directory;

        void assimp_load(const std::string& path);
        void assimp_node_process(aiNode* node, const aiScene* scene);
        Mesh* assimp_mesh_process(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture*> assimp_textures_load(aiMaterial* mat, aiTextureType type);

        void protobuf_load(const std::string& path);

    public:
        void render(Shader& shader);
        void fill_protobuf(glp_util::Model* pb);

        Model(const std::string& path, bool assimp);
        ~Model();
};
