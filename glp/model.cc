#include <cassert>
#include <cstddef>
#include <fstream>
#include <google/protobuf/stubs/common.h>
#include <string>

#include "model.hh"
#include "utils.hh"
#include "../utils/model.pb.h"

constexpr GLuint POSITION_ATTRIBUTE_INDEX       = 0;
constexpr GLuint NORMAL_ATTRIBUTE_INDEX         = 1;
constexpr GLuint TEXCOORD0_ATTRIBUTE_INDEX      = 2;
constexpr GLuint JOINTS_ATTRIBUTE_INDEX         = 3;
constexpr GLuint WEIGHTS_ATTRIBUTE_INDEX        = 4;

Mesh::Mesh(std::vector<Vertex> vert, std::vector<unsigned int> idx, std::vector<Texture*> tex)
    : vertices{vert}, indices{idx}, textures{tex} {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex),
            vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int),
            indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX,
            3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);

    glVertexAttribPointer(NORMAL_ATTRIBUTE_INDEX,
            3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);

    glVertexAttribPointer(TEXCOORD0_ATTRIBUTE_INDEX,
            2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(TEXCOORD0_ATTRIBUTE_INDEX);

    glBindVertexArray(0);
}

void Mesh::render(Shader& shader) {
    for(uint32_t i=0; i<textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        shader.set("tex" + std::to_string(i), (int)i);
        glBindTexture(GL_TEXTURE_2D, textures[i]->id);
    }
    glActiveTexture(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    for(auto& tex: textures) delete tex;
}

Model::Model(const std::string& path, bool assimp) {
    if(assimp) assimp_load(path);
    else protobuf_load(path);
}

void Model::assimp_load(const std::string& path) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        glp_logv("assimp returned %s", import.GetErrorString());
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    assimp_node_process(scene->mRootNode, scene);
}

void Model::assimp_node_process(aiNode* node, const aiScene* scene) {
    for(size_t i=0; i<node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        meshes.push_back(assimp_mesh_process(mesh, scene));
    }

    for(size_t i=0; i<node->mNumChildren; i++)
        assimp_node_process(node->mChildren[i], scene);
}

Mesh* Model::assimp_mesh_process(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> idxs;
    std::vector<Texture*> texs;

    for(size_t i=0; i<mesh->mNumVertices; i++) {
        Vertex vertex {
            glm::vec3(mesh->mVertices[i].x,
                    mesh->mVertices[i].y,
                    mesh->mVertices[i].z),
            mesh->mNormals ?
                glm::vec3(mesh->mNormals[i].x,
                        mesh->mNormals[i].y,
                        mesh->mNormals[i].z) :
                glm::vec3(0.0f, 0.0f, 0.0f),
            mesh->mTextureCoords[0] ? 
                glm::vec2(mesh->mTextureCoords[0][i].x,
                        mesh->mTextureCoords[0][i].y) :
                glm::vec2(0.0f, 0.0f),
        };
        verts.push_back(vertex);
    }

    for(size_t i=0; i<mesh->mNumFaces; i++) {
        aiFace* face = &mesh->mFaces[i];
        for(size_t j=0; j<face->mNumIndices; j++)
            idxs.push_back(face->mIndices[j]);
    }

    if(mesh->mMaterialIndex >= 0) {
        auto material = scene->mMaterials[mesh->mMaterialIndex];

        auto diffuse = assimp_textures_load(material, aiTextureType_DIFFUSE);
        texs.insert(texs.end(), diffuse.begin(), diffuse.end());
    }
    
    return new Mesh(verts, idxs, texs);
}

std::vector<Texture*> Model::assimp_textures_load(aiMaterial* mat, aiTextureType type) {
    std::vector<Texture*> texs;

    for(size_t i=0; i<mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string full_path = directory + '/' + str.C_Str();
        auto tex = new Texture{full_path};
        texs.push_back(tex);
    }

    return texs;
}

void Model::render(Shader& shader) {
    for(auto& mesh: meshes) mesh->render(shader);
}

Model::~Model() {
    for(auto& mesh: meshes) delete mesh;
}


void Model::fill_protobuf(glp_util::Model* pb) {
    for(const auto& mesh: meshes) {
        auto pb_mesh = pb->add_meshes();

        for(const auto& vert: mesh->vertices) {
            auto pb_vert = pb_mesh->add_vertices();
            auto pb_pos = pb_vert->mutable_position();
            pb_pos->set_x(vert.position.x);
            pb_pos->set_y(vert.position.y);
            pb_pos->set_z(vert.position.z);
            auto pb_norm = pb_vert->mutable_normal();
            pb_norm->set_x(vert.normal.x);
            pb_norm->set_y(vert.normal.y);
            pb_norm->set_z(vert.normal.z);
            auto pb_uv = pb_vert->mutable_uv();
            pb_uv->set_x(vert.uv.x);
            pb_uv->set_y(vert.uv.y);
        }

        for(const auto& idx: mesh->indices)
            pb_mesh->add_indices(idx);

        for(const auto& tex: mesh->textures)
            pb_mesh->add_textures(tex->path);
    }
}

void Model::protobuf_load(const std::string& path) {
    GOOGLE_PROTOBUF_VERIFY_VERSION;

    glp_util::Model model;

    std::fstream input(path, std::ios::in | std::ios::binary);
    if(!model.ParseFromIstream(&input)) {
        glp_logv("failed to load model from protobuf %s", path.c_str());
        return;
    }

    for(size_t i=0; i<model.meshes_size(); i++) {
        auto pb_mesh = model.meshes(i);
        std::vector<Vertex> vertices;
        std::vector<unsigned int> idxs;
        std::vector<Texture*> texs;

        for(size_t j=0; j<pb_mesh.vertices_size(); j++) {
            auto pb_vert = pb_mesh.vertices(j);
            auto pb_pos = pb_vert.position();
            auto pb_norm = pb_vert.normal();
            auto pb_uv = pb_vert.uv();
            vertices.emplace_back(
                    glm::vec3(pb_pos.x(), pb_pos.y(), pb_pos.z()),
                    glm::vec3(pb_norm.x(), pb_norm.y(), pb_norm.z()),
                    glm::vec2(pb_uv.x(), pb_uv.y()));
        }

        for(size_t j=0; j<pb_mesh.indices_size(); j++) {
            auto pb_idx = pb_mesh.indices(j);
            idxs.push_back(pb_idx);
        }

        for(size_t j=0; j<pb_mesh.textures_size(); j++) {
            glp_logv("%s", pb_mesh.textures(j).c_str());
            auto tex = new Texture{pb_mesh.textures(j)};
            texs.push_back(tex);
        }

        auto mesh = new Mesh(vertices, idxs, texs);
        meshes.push_back(mesh);
    }

    google::protobuf::ShutdownProtobufLibrary();
}
