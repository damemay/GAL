#include <cassert>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <limits>

#include "material.hh"
#include "model.hh"
#include "utils.hh"

namespace glp {

Mesh::Mesh(std::vector<Vertex> vert, std::vector<unsigned int> idx, Material* mat, Shader* shader)
    : vertices{vert}, indices{idx}, material{mat} {
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

    glVertexAttribPointer(JOINTS_ATTRIBUTE_INDEX,
            4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bone_index));
    glEnableVertexAttribArray(JOINTS_ATTRIBUTE_INDEX);

    glVertexAttribPointer(WEIGHTS_ATTRIBUTE_INDEX,
            4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
    glEnableVertexAttribArray(WEIGHTS_ATTRIBUTE_INDEX);

#ifdef __vita__
    shader->bind();
    GLuint id = glGetAttribLocation(shader->get(), "position");
    glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(id);

    id = glGetAttribLocation(shader->get(), "normal");
    glVertexAttribPointer(id, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(id);

    id = glGetAttribLocation(shader->get(), "texcoord0");
    glVertexAttribPointer(id, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(id);

    id = glGetAttribLocation(shader->get(), "joints");
    glVertexAttribPointer(id, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bone_index));
    glEnableVertexAttribArray(id);

    id = glGetAttribLocation(shader->get(), "weights");
    glVertexAttribPointer(id, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
    glEnableVertexAttribArray(id);
    shader->unbind();
#endif

    glBindVertexArray(0);
}

void Mesh::render(Shader* shader, ShadingType type) {
    uint8_t count = 0;
    if(material->diffuse_id>-1) {
        glActiveTexture(GL_TEXTURE0+count);
        shader->set("material.diffuse_tex_exists", 1);
        shader->set("material.diffuse_tex", count);
        glBindTexture(GL_TEXTURE_2D, material->textures[material->diffuse_id]->id);
        count++;
    } else {
        shader->set("material.diffuse_tex_exists", 0);
        if(type==ShadingType::PHONG) {
            shader->set("material.diffuse", material->diffuse);
            shader->set("material.ambient", material->ambient);
            shader->set("material.shininess", material->shininess);
        } else shader->set("material.albedo", material->albedo);
    }

    if(material->normal_id>-1) {
        glActiveTexture(GL_TEXTURE0+count);
        shader->set("material.normal_tex_exists", 1);
        shader->set("material.normal_tex", count);
        glBindTexture(GL_TEXTURE_2D, material->textures[material->normal_id]->id);
        count++;
    } else shader->set("material.normal_tex_exists", 0);

    if(type == ShadingType::PHONG) {
        if(material->specular_id>-1) {
            glActiveTexture(GL_TEXTURE0+count);
            shader->set("material.specular_tex_exists", 1);
            shader->set("material.specular_tex", count);
            glBindTexture(GL_TEXTURE_2D, material->textures[material->specular_id]->id);
            count++;
        } else {
            shader->set("material.specular_tex_exists", 0);
            shader->set("material.specular", material->specular);
        }
    } else {
        if(material->metallic_id>-1) {
            glActiveTexture(GL_TEXTURE0+count);
            shader->set("material.metallic_tex_exists", 1);
            shader->set("material.metallic_tex", count);
            glBindTexture(GL_TEXTURE_2D, material->textures[material->metallic_id]->id);
            count++;
        } else {
            shader->set("material.metallic_tex_exists", 0);
            shader->set("material.metallic", material->metallic);
        }

        if(material->roughness_id>-1) {
            glActiveTexture(GL_TEXTURE0+count);
            shader->set("material.roughness_tex_exists", 1);
            shader->set("material.roughness_tex", count);
            glBindTexture(GL_TEXTURE_2D, material->textures[material->roughness_id]->id);
            count++;
        } else {
            shader->set("material.roughness_tex_exists", 0);
            shader->set("material.roughness", material->roughness);
        }

        if(material->ao_id>-1) {
            glActiveTexture(GL_TEXTURE0+count);
            shader->set("material.ao_tex_exists", 1);
            shader->set("material.ao_tex", count);
            glBindTexture(GL_TEXTURE_2D, material->textures[material->ao_id]->id);
            count++;
        } else {
            shader->set("material.ao_tex_exists", 0);
        }
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

Model::Model(const std::string& path, Shader* shader_, ShadingType shading_t) 
    : shader{shader_}, shading{shading_t} {
    load(path);
}

void Model::load(const std::string& path) {
#ifdef USE_ASSIMP
    assimp_load(path);
#else
    auto file = util::read_file(path);
    directory = path.substr(0, path.find_last_of('/'));
    auto decompressed = util::decompress(file);
    std::stringstream s;
    s << decompressed;
    deserialize_data(s);
#endif
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

Texture* Model::assimp_textures_load(aiMaterial* mat, aiTextureType type) {
    Texture* tex {nullptr};
    for(size_t i=0; i<mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);
        std::string full_path = directory + '/' + str.C_Str();
        tex = texture_load(full_path);
    }
    return tex;
}

Mesh* Model::assimp_mesh_process(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> verts;
    std::vector<unsigned int> idxs;
    Material* mat {nullptr};

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
        mat = new Material;
        auto aimat = scene->mMaterials[mesh->mMaterialIndex];
        {
            aiColor3D diffuse;
            aimat->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse);
            mat->diffuse = glm::vec3(diffuse.r, diffuse.g, diffuse.b);
        }{
            aiColor3D ambient;
            aimat->Get(AI_MATKEY_COLOR_AMBIENT, ambient);
            mat->ambient = glm::vec3(ambient.r, ambient.g, ambient.b);
        }{
            aiColor3D specular;
            aimat->Get(AI_MATKEY_COLOR_SPECULAR, specular);
            mat->specular = glm::vec3(specular.r, specular.g, specular.b);
        }{
            ai_real shininess;
            aimat->Get(AI_MATKEY_SHININESS, shininess);
            mat->shininess = shininess;
        }{
            aiColor3D base;
            aimat->Get(AI_MATKEY_BASE_COLOR, base);
            mat->albedo = glm::vec3(base.r, base.g, base.b);
        }{
            ai_real metallic;
            aimat->Get(AI_MATKEY_METALLIC_FACTOR, metallic);
            mat->metallic = metallic;
            glp_logv("%f", metallic);
        }{
            ai_real roughness;
            aimat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness);
            mat->roughness = roughness;
        }

        int8_t count = 0;
        auto dif = assimp_textures_load(aimat, aiTextureType_DIFFUSE);
        if(dif) mat->textures.push_back(dif), mat->diffuse_id = count++;
        auto spec = assimp_textures_load(aimat, aiTextureType_SPECULAR);
        if(spec) mat->textures.push_back(spec), mat->specular_id = count++;

        auto rough = assimp_textures_load(aimat, aiTextureType_DIFFUSE_ROUGHNESS);
        if(rough) mat->textures.push_back(rough), mat->roughness_id = count++;
        auto metal = assimp_textures_load(aimat, aiTextureType_METALNESS);
        if(metal) mat->textures.push_back(metal), mat->metallic_id = count++;
        auto ao = assimp_textures_load(aimat, aiTextureType_AMBIENT_OCCLUSION);
        if(ao) mat->textures.push_back(ao), mat->ao_id = count++;
        auto normal = assimp_textures_load(aimat, aiTextureType_NORMALS);
        if(normal) mat->textures.push_back(normal), mat->normal_id = count++;
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
    
    return new Mesh(verts, idxs, mat, shader);
}
#endif

void Model::render() {
    for(auto& mesh: meshes) mesh->render(shader, shading);
}

glm::vec3 Model::calculate_bounding_box() {
    glm::vec3 min {std::numeric_limits<float>::max()};
    glm::vec3 max {-std::numeric_limits<float>::max()};
    for(const auto& mesh: meshes) {
        for(const auto& vert: mesh->vertices) {
            min.x = std::min(min.x, vert.position.x);
            min.y = std::min(min.y, vert.position.y);
            min.z = std::min(min.z, vert.position.z);
            max.x = std::max(max.x, vert.position.x);
            max.y = std::max(max.y, vert.position.y);
            max.z = std::max(max.z, vert.position.z);
        }
    }
    return glm::vec3((max.x-min.x)/2,
            (max.y-min.y)/2,
            (max.z-min.z)/2);
}

std::vector<glm::vec3> Model::calculate_bounding_boxes() {
    std::vector<glm::vec3> boxes;
    for(const auto& mesh: meshes) {
        glm::vec3 min {std::numeric_limits<float>::max()};
        glm::vec3 max {-std::numeric_limits<float>::max()};
        for(const auto& vert: mesh->vertices) {
            min.x = std::min(min.x, vert.position.x);
            min.y = std::min(min.y, vert.position.y);
            min.z = std::min(min.z, vert.position.z);
            max.x = std::max(max.x, vert.position.x);
            max.y = std::max(max.y, vert.position.y);
            max.z = std::max(max.z, vert.position.z);
        }
        boxes.emplace_back((max.x-min.x)/2,
                (max.y-min.y)/2,
                (max.z-min.z)/2);
    }
    return boxes;
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
        s << "mat " << "texs " << mesh->material->textures.size() << ' ';
        for(const auto& tex: mesh->material->textures) {
            size_t f = tex->path.find_last_of("/");
            s << tex->path.substr(f+1) << ' ';
        }
        s << "amb " << mesh->material->ambient.x << ' '
            << mesh->material->ambient.y << ' '
            << mesh->material->ambient.z << ' ';
        s << "dif " << mesh->material->diffuse.x << ' '
            << mesh->material->diffuse.y << ' '
            << mesh->material->diffuse.z << ' ';
        s << "spc " << mesh->material->specular.x << ' '
            << mesh->material->specular.y << ' '
            << mesh->material->specular.z << ' ';
        s << "shn " << mesh->material->shininess << ' ';
        s << "alb " << mesh->material->albedo.x << ' '
            << mesh->material->albedo.y << ' '
            << mesh->material->albedo.z << ' ';
        s << "mtl " << mesh->material->metallic << ' ';
        s << "rgh " << mesh->material->roughness << ' ';
        s << "did " << mesh->material->diffuse_id << ' ';
        s << "sid " << mesh->material->specular_id << ' ';
        s << "mid " << mesh->material->metallic_id << ' ';
        s << "rid " << mesh->material->roughness_id << ' ';
        s << "aid " << mesh->material->ao_id << ' ';
        s << "nid " << mesh->material->normal_id << ' ';
       // s << "texs " << mesh->textures.size() << ' ';
       // for(const auto& tex: mesh->textures)
       //     s << tex->path << ' ';
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
        s >> name; assert(name == "mat");
        auto mat = new Material;
        s >> name; assert(name == "texs");
        s >> count;
        mat->textures.resize(count);
        for(size_t i=0; i<mat->textures.size(); i++) {
            s >> name;
            mat->textures[i] = texture_load(directory + '/' + name);
        }
        s >> name; assert(name == "amb");
        s >> mat->ambient.x
            >> mat->ambient.y
            >> mat->ambient.z;
        s >> name; assert(name == "dif");
        s >>   mat->diffuse.x
            >> mat->diffuse.y
            >> mat->diffuse.z;
        s >> name; assert(name == "spc");
        s >>   mat->specular.x
            >> mat->specular.y
            >> mat->specular.z;
        s >> name; assert(name == "shn");
        s >>   mat->shininess;
        s >> name; assert(name == "alb");
        s >>   mat->albedo.x
            >> mat->albedo.y
            >> mat->albedo.z;
        s >> name; assert(name == "mtl");
        s >>   mat->metallic;
        s >> name; assert(name == "rgh");
        s >>   mat->roughness;
        s >> name; assert(name == "did");
        s >>   mat->diffuse_id;
        s >> name; assert(name == "sid");
        s >>   mat->specular_id;
        s >> name; assert(name == "mid");
        s >>   mat->metallic_id;
        s >> name; assert(name == "rid");
        s >>   mat->roughness_id;
        s >> name; assert(name == "aid");
        s >>   mat->ao_id;
        s >> name; assert(name == "nid");
        s >>   mat->normal_id;
        meshes[i] = new Mesh(verts, idxs, mat, shader);
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

}
