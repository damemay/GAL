#include <cassert>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>

#include "model.hh"
#include "utils.hh"

Mesh::Mesh(std::vector<Vertex> vert, std::vector<unsigned int> idx, std::vector<Texture*> tex, Shader* shader)
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

#ifndef __vita__
    glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX,
            3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);

    glVertexAttribPointer(NORMAL_ATTRIBUTE_INDEX,
            3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);

    glVertexAttribPointer(TEXCOORD0_ATTRIBUTE_INDEX,
            2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(TEXCOORD0_ATTRIBUTE_INDEX);

    glVertexAttribPointer(JOINTS_ATTRIBUTE_INDEX,
            4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bone_index));
    glEnableVertexAttribArray(JOINTS_ATTRIBUTE_INDEX);

    glVertexAttribPointer(WEIGHTS_ATTRIBUTE_INDEX,
            4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
    glEnableVertexAttribArray(WEIGHTS_ATTRIBUTE_INDEX);
#else
    shader->bind();
    GLuint id = glGetAttribLocation(shader->get(), "position");
    glp_logv("position %d", id);
    glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(id);
    id = glGetAttribLocation(shader->get(), "normal");
    glp_logv("normal %d", id);
    glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(id);
    id = glGetAttribLocation(shader->get(), "texcoord0");
    glp_logv("texcoord0 %d", id);
    glVertexAttribPointer(id, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(id);
    id = glGetAttribLocation(shader->get(), "joints");
    glp_logv("joints %d", id);
    glVertexAttribPointer(id, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bone_index));
    glEnableVertexAttribArray(id);
    id = glGetAttribLocation(shader->get(), "weights");
    glp_logv("weights %d", id);
    glVertexAttribPointer(id, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
    glEnableVertexAttribArray(id);
    shader->unbind();
#endif

    glBindVertexArray(0);
}

void Mesh::render(Shader* shader) {
    for(int i=0; i<textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        shader->set("tex" + std::to_string(i), i);
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
}

#ifdef USE_ASSIMP
Model::Model(const std::string& path, bool assimp, Shader* shader_) 
    : shader{shader_} {
    if(assimp) assimp_load(path);
    else {
        std::fstream out(path, std::ios::in);
        if(!out) glp_logv("error opening file %s", path.c_str());
        std::stringstream s;
        s << out.rdbuf();
        deserialize_data(s);
    }
}
#endif

Model::Model(const std::string& path, Shader* shader_) 
    : shader{shader_} {
    std::fstream out(path, std::ios::in);
    if(!out) glp_logv("error opening file %s", path.c_str());
    std::stringstream s;
    s << out.rdbuf();
    auto decompressed = util::decompress(s.str());
    s.str("");
    s << decompressed;
    deserialize_data(s);
}

#ifdef USE_ASSIMP
void Model::assimp_load(const std::string& path) {
    Assimp::Importer import;
    const aiScene* scene = import.ReadFile(path, aiProcess_JoinIdenticalVertices | aiProcess_Triangulate | aiProcess_FlipUVs);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        glp_logv("assimp returned %s", import.GetErrorString());
        return;
    }
    directory = path.substr(0, path.find_last_of('/'));

    assimp_node_process(scene->mRootNode, scene);

    glp_logv("model bone count: %lu", bones.size());
}

std::vector<Texture*> Model::assimp_textures_load(aiMaterial* mat, aiTextureType type) {
    std::vector<Texture*> texs;
    for(size_t i=0; i<mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string full_path = directory + '/' + str.C_Str();
        Texture* tex = texture_load(full_path);
        texs.push_back(tex);
    }
    return texs;
}
#endif

Texture* Model::texture_load(const std::string& path) {
    Texture* tex;
    bool skip = false;
    for(Texture* loaded: textures) {
        if(loaded->path == path) {
            tex = loaded;
            skip = true;
            break;
        }
    }
    if(!skip) {
        tex = new Texture{path};
        glp_logv("new texture: %s", tex->path.c_str());
        textures.push_back(tex);
    }
    return tex;
}

#ifdef USE_ASSIMP
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
    std::vector<Texture*> textures;

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
                {0.0f, 0.0f, 0.0f, 0.0f},
                {0.0f, 0.0f, 0.0f, 0.0f},
        };
        verts.push_back(vertex);
    }

    for(size_t i=0; i<mesh->mNumFaces; i++) {
        aiFace* face = &mesh->mFaces[i];
        for(size_t j=0; j<face->mNumIndices; j++)
            idxs.push_back(face->mIndices[j]);
    }

    if(mesh->mMaterialIndex >= 0) {
        auto tex = assimp_textures_load(scene->mMaterials[mesh->mMaterialIndex], aiTextureType_DIFFUSE);
        textures.insert(textures.end(), tex.begin(), tex.end());
    }


    if(mesh->HasBones()) {
        for(size_t i=0; i<mesh->mNumBones; i++) {
            auto ai_bone = mesh->mBones[i];
            std::string name = ai_bone->mName.C_Str();
            bool exists = false;

            for(const auto& bone: bones)
                if(name == bone.name) exists = true;

            if(!exists) {
                bones.emplace_back(name, glm::mat4(
                            ai_bone->mOffsetMatrix.a1, ai_bone->mOffsetMatrix.b1,
                            ai_bone->mOffsetMatrix.c1, ai_bone->mOffsetMatrix.d1,
                            ai_bone->mOffsetMatrix.a2, ai_bone->mOffsetMatrix.b2,
                            ai_bone->mOffsetMatrix.c2, ai_bone->mOffsetMatrix.d2,
                            ai_bone->mOffsetMatrix.a3, ai_bone->mOffsetMatrix.b3,
                            ai_bone->mOffsetMatrix.c3, ai_bone->mOffsetMatrix.d3,
                            ai_bone->mOffsetMatrix.a4, ai_bone->mOffsetMatrix.b4,
                            ai_bone->mOffsetMatrix.c4, ai_bone->mOffsetMatrix.d4));
            }

            for(size_t j=0; j<ai_bone->mNumWeights; j++) {
                auto weight = ai_bone->mWeights[j];
                auto v = weight.mVertexId;
                for(size_t k=0; k<MAX_BONE_INFLUENCE; k++) {
                    if(verts[v].weights[k]==0.0f) {
                        verts[v].bone_index[k] = i;
                        verts[v].weights[k] = weight.mWeight;
                    }
                }
            }
        }
    }
    
    return new Mesh(verts, idxs, textures, shader);
}
#endif

void Model::render() {
    for(auto& mesh: meshes) mesh->render(shader);
}

Model::~Model() {
    for(auto& mesh: meshes) delete mesh;
    for(auto& tex: textures) delete tex;
}

std::stringstream Model::serialize_data() {
    std::stringstream s;
    s << "meshes " << meshes.size() << ' ';
    for(const auto& mesh: meshes) {
        s << "mesh " << "verts " << mesh->vertices.size() << ' ';
        for(const auto& vert: mesh->vertices) {
            s << "vert ";
            s << "p " << vert.position.x << ' '
                << vert.position.y << ' '
                << vert.position.z << ' ';
            s << "n " << vert.normal.x << ' '
                << vert.normal.y << ' '
                << vert.normal.z << ' ';
            s << "u " << vert.uv.x << ' '
                << vert.uv.y << ' ' << ' ';
            s << "b " << vert.bone_index[0] << ' '
                << vert.bone_index[1] << ' '
                << vert.bone_index[2] << ' '
                << vert.bone_index[3] << ' ';
            s << "w " << vert.weights[0] << ' '
                << vert.weights[1] << ' '
                << vert.weights[2] << ' '
                << vert.weights[3] << ' ';
        }
        s << "idxs " << mesh->indices.size() << ' ';
        for(const auto& id: mesh->indices)
            s << id << ' ';
        s << "texs " << mesh->textures.size() << ' ';
        for(const auto& tex: mesh->textures)
            s << tex->path << ' ';
    }

    s << "bones " << bones.size() << ' ';
    for(const auto& bone: bones) {
        s << "b " << bone.name << ' ';
        s << bone.offset[0].x << ' '
            << bone.offset[0].y << ' '
            << bone.offset[0].z << ' '
            << bone.offset[0].w << ' '
            << bone.offset[1].x << ' '
            << bone.offset[1].y << ' '
            << bone.offset[1].z << ' '
            << bone.offset[1].w << ' '
            << bone.offset[2].x << ' '
            << bone.offset[2].y << ' '
            << bone.offset[2].z << ' '
            << bone.offset[2].w << ' '
            << bone.offset[3].x << ' '
            << bone.offset[3].y << ' '
            << bone.offset[3].z << ' '
            << bone.offset[3].w << ' ';
    }

    return s;
}

void Model::deserialize_data(std::stringstream& s) {
    std::string name;
    size_t count;
    
    s >> name >> count; assert("meshes");
    if(count > 0) meshes.resize(count);
    
    for(size_t i=0; i<meshes.size(); i++) {
        s >> name; assert(name == "mesh");
        s >> name; assert(name == "verts");
        s >> count;
        std::vector<Vertex> verts;
        if(count > 0) verts.resize(count);
        for(size_t i=0; i<verts.size(); i++) {
            s >> name; assert(name == "vert");
            s >> name; assert(name == "p");
            s >> verts[i].position.x
                >> verts[i].position.y
                >> verts[i].position.z;
            s >> name; assert(name == "n");
            s >> verts[i].normal.x
                >> verts[i].normal.y
                >> verts[i].normal.z;
            s >> name; assert(name == "u");
            s >> verts[i].uv.x
                >> verts[i].uv.y;
            s >> name; assert(name == "b");
            s >> verts[i].bone_index[0]
                >> verts[i].bone_index[1]
                >> verts[i].bone_index[2]
                >> verts[i].bone_index[3];
            s >> name; assert(name == "w");
            s >> verts[i].weights[0]
                >> verts[i].weights[1]
                >> verts[i].weights[2]
                >> verts[i].weights[3];
        }
        s >> name; assert(name == "idxs");
        s >> count;
        std::vector<unsigned int> idxs;
        if(count > 0) idxs.resize(count);
        for(size_t i=0; i<idxs.size(); i++)
            s >> idxs[i];
        s >> name; assert(name == "texs");
        s >> count;
        std::vector<Texture*> texs;
        texs.resize(count);
        for(size_t i=0; i<texs.size(); i++) {
            s >> name;
            texs[i] = texture_load(name);
        }
        meshes[i] = new Mesh(verts, idxs, texs, shader);
    }

    s >> name; assert("bones");
    s >> count;
    if(count > 0) bones.resize(count);
    for(size_t i=0; i<bones.size(); i++) {
        s >> name; assert("b");
        s >> bones[i].name;
        glm::mat4 to;
        s >> to[0].x >> to[0].y >> to[0].z >> to[0].w
            >> to[1].x >> to[1].y >> to[1].z >> to[1].w
            >> to[2].x >> to[2].y >> to[2].z >> to[2].w
            >> to[3].x >> to[3].y >> to[3].z >> to[3].w;
        bones[i].offset = to;
    }
}
