#include <cassert>
#include <cstddef>
#include <fstream>
#include <string>

#include "model.hh"
#include "utils.hh"
#include "../utils/model.pb.h"

SkeletonNode* SkeletonNode::find(std::string name_) {
    if(name == name_) return this;
    for(auto& child: children)
        if(auto s = child.find(name)) return s;

    return nullptr;
}

Mesh::Mesh(std::vector<Vertex> vert, std::vector<unsigned int> idx)
    : vertices{vert}, indices{idx} {
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

    glBindVertexArray(0);
}

void Mesh::render(Shader& shader) {

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

Model::Model(const std::string& path, bool assimp) {
    if(assimp) assimp_load(path);
    // else protobuf_load(path);
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

    glp_logv("model bone count: %lu", bones.size());

    if(scene->HasMaterials()) {
        for(size_t i=0; i<scene->mNumMaterials; i++) {
            auto tex = assimp_textures_load(scene->mMaterials[i], aiTextureType_DIFFUSE);
            textures.insert(textures.end(), tex.begin(), tex.end());
        }
    }

    glp_logv("model texture count: %lu", textures.size());

    if(!assimp_skeleton_import(scene->mRootNode, root_node))
        glp_log("model without skeleton");

    if(scene->HasAnimations())
        for(size_t i=0; i<scene->mNumAnimations; i++)
            assimp_animation_load(scene->mAnimations[i]);

    glp_logv("model animation count: %lu", animations.size());

}

int Model::assimp_skeleton_import(aiNode* ai_node, SkeletonNode& parent) {
    SkeletonNode t_node{};
    t_node.name = ai_node->mName.C_Str();

    bool has_bone = false;
    for(size_t i=0; i<bones.size(); i++) {
        if(bones[i].name == t_node.name) {
            t_node.bone_index = i;
            has_bone = true;
            break;
        }
    }

    bool has_usable_child = false;
    t_node.children.resize(ai_node->mNumChildren);
    for(size_t i=0; i<ai_node->mNumChildren; i++) {
        if(assimp_skeleton_import(ai_node->mChildren[i], t_node.children[i])) {
            has_usable_child = true;
        }
    }

    if(has_usable_child || has_bone) {
        parent = t_node;
        return 1;
    }

    return 0;
}

void Model::assimp_animation_load(aiAnimation* ai_anim) {
    Animation anim;
    anim.name = ai_anim->mName.C_Str();
    anim.duration = ai_anim->mDuration;
    anim.ticks_per_second = ai_anim->mTicksPerSecond;
    glp_logv("found new animation in model: %s", anim.name.c_str());
    
    for(size_t j=0; j<ai_anim->mNumChannels; j++) {
        auto ai_chan = ai_anim->mChannels[j];
        AnimationChannel channel;
        channel.node_name = ai_chan->mNodeName.C_Str();
    
        for(size_t k=0; k<ai_chan->mNumPositionKeys; k++)
            channel.position_keys.emplace_back(glm::vec3(
                        ai_chan->mPositionKeys[k].mValue.x,
                        ai_chan->mPositionKeys[k].mValue.y,
                        ai_chan->mPositionKeys[k].mValue.z),
                    ai_chan->mPositionKeys[k].mTime);
    
        for(size_t k=0; k<ai_chan->mNumRotationKeys; k++)
            channel.rotation_keys.emplace_back(glm::quat(
                        ai_chan->mRotationKeys[k].mValue.x,
                        ai_chan->mRotationKeys[k].mValue.y,
                        ai_chan->mRotationKeys[k].mValue.z,
                        ai_chan->mRotationKeys[k].mValue.w),
                    ai_chan->mRotationKeys[k].mTime);
    
        for(size_t k=0; k<ai_chan->mNumScalingKeys; k++)
            channel.scale_keys.emplace_back(glm::vec3(
                        ai_chan->mScalingKeys[k].mValue.x,
                        ai_chan->mScalingKeys[k].mValue.y,
                        ai_chan->mScalingKeys[k].mValue.z),
                    ai_chan->mScalingKeys[k].mTime);
    }
    
    animations.push_back(anim);

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

    if(mesh->HasBones()) {
        for(size_t i=0; i<mesh->mNumBones; i++) {
            auto ai_bone = mesh->mBones[i];
            std::string name = ai_bone->mName.C_Str();
            bool exists = false;

            for(const auto& bone: bones)
                if(name == bone.name) exists = true;

            if(!exists) {
                bones.emplace_back(name, glm::mat4(
                            ai_bone->mOffsetMatrix.a1, ai_bone->mOffsetMatrix.b1, ai_bone->mOffsetMatrix.c1, ai_bone->mOffsetMatrix.d1,
                            ai_bone->mOffsetMatrix.a2, ai_bone->mOffsetMatrix.b2, ai_bone->mOffsetMatrix.c2, ai_bone->mOffsetMatrix.d2,
                            ai_bone->mOffsetMatrix.a3, ai_bone->mOffsetMatrix.b3, ai_bone->mOffsetMatrix.c3, ai_bone->mOffsetMatrix.d3,
                            ai_bone->mOffsetMatrix.a4, ai_bone->mOffsetMatrix.b4, ai_bone->mOffsetMatrix.c4, ai_bone->mOffsetMatrix.d4));
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
    
    return new Mesh(verts, idxs);
}

void Model::animation_load(const Animation& animation) {
    animation_bones.resize(bones.size());
    for(const auto& chan: animation.channels) {
        SkeletonNode* node = root_node.find(chan.node_name);
        node->keys.position_keys = chan.position_keys;
        node->keys.rotation_keys = chan.rotation_keys;
        node->keys.scale_keys = chan.scale_keys;
    }
}

glm::mat4 Model::animation_translate(const SkeletonNode& node, const Animation& animation) {
    auto mat = glm::mat4(1.0f);
    if(node.keys.position_keys.size() > 0) {
        size_t p_key = 0, n_key = 0;
        for(size_t i=0; i<node.keys.position_keys.size()-1; ++i) {
            p_key = i;
            n_key = i+1;
            if(node.keys.position_keys[n_key].time >= animation.duration) break;
        }
        
        float t_tot = node.keys.position_keys[n_key].time - node.keys.position_keys[p_key].time;
        float t = (animation.duration - node.keys.position_keys[p_key].time) / t_tot;
        auto vi = node.keys.position_keys[p_key].value;
        auto vf = node.keys.position_keys[n_key].value;
        auto lerp0 = vi * (1.0f-t);
        auto lerp1 = vf * t;
        auto lerp = lerp0 + lerp1;
        mat = glm::translate(mat, lerp);
    }
    return mat;
}

glm::mat4 Model::animation_rotate(const SkeletonNode& node, const Animation& animation) {
    auto mat = glm::mat4(1.0f);
    if(node.keys.rotation_keys.size() > 0) {
        size_t p_key = 0, n_key = 0;
        for(size_t i=0; i<node.keys.rotation_keys.size()-1; ++i) {
            p_key = i;
            n_key = i+1;
            if(node.keys.rotation_keys[n_key].time >= animation.duration) break;
        }
        
        float t_tot = node.keys.rotation_keys[n_key].time - node.keys.rotation_keys[p_key].time;
        float t = (animation.duration - node.keys.rotation_keys[p_key].time) / t_tot;
        auto vi = node.keys.rotation_keys[p_key].value;
        auto vf = node.keys.rotation_keys[n_key].value;
        auto lerp = glm::slerp(vi, vf, t);
        mat = glm::mat4(lerp);
    }
    return mat;
}

glm::mat4 Model::animation_scale(const SkeletonNode& node, const Animation& animation) {
    auto mat = glm::mat4(1.0f);
    if(node.keys.scale_keys.size() > 0) {
        size_t p_key = 0, n_key = 0;
        for(size_t i=0; i<node.keys.scale_keys.size()-1; ++i) {
            p_key = i;
            n_key = i+1;
            if(node.keys.scale_keys[n_key].time >= animation.duration) break;
        }
        
        float t_tot = node.keys.scale_keys[n_key].time - node.keys.scale_keys[p_key].time;
        float t = (animation.duration - node.keys.scale_keys[p_key].time) / t_tot;
        auto vi = node.keys.scale_keys[p_key].value;
        auto vf = node.keys.scale_keys[n_key].value;
        auto lerp0 = vi * (1.0f-t);
        auto lerp1 = vf * t;
        auto lerp = lerp0 + lerp1;
        mat = glm::translate(mat, lerp);
    }
    return mat;
}

void Model::animation(const SkeletonNode& node, const Animation& anim) {
    auto out = glm::mat4(1.0f);
    auto t = animation_translate(node, anim);
    auto r = animation_rotate(node, anim);
    auto s = animation_scale(node, anim);
    glm::mat4 loc = t*r*s;
    int bone = node.bone_index;
    if(bone > -1) {
        auto bone_off = bones[bone].matrix;
        out = out * loc * bone_off;
    }
    for(const auto& child: node.children)
        animation(child, anim);
}

void Model::animate(uint32_t index) {
    auto& anim = animations[index];
    if(index > animations.size()) return;
    animation_load(anim);
    animation(root_node, anim);
}

void Model::animate(const std::string& name) {
    auto it = std::find_if(animations.begin(), animations.end(),
            [name](const Animation& anim) { return anim.name == name;});
    if(it != animations.end()) {
        auto index = std::distance(animations.begin(), it);
        auto& anim = animations[index];
        animation_load(anim);
        animation(root_node, anim);
    }
}

void Model::render(Shader& shader) {
    for(uint32_t i=0; i<textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        shader.set("tex" + std::to_string(i), (int)i);
        glBindTexture(GL_TEXTURE_2D, textures[i]->id);
    }
    glActiveTexture(GL_TEXTURE0);
    for(auto& mesh: meshes) mesh->render(shader);
}

Model::~Model() {
    for(auto& mesh: meshes) delete mesh;
    for(auto& tex: textures) delete tex;
}


//void Model::fill_protobuf(glp_util::Model* pb) {
//    for(const auto& mesh: meshes) {
//        auto pb_mesh = pb->add_meshes();
//
//        for(const auto& vert: mesh->vertices) {
//            auto pb_vert = pb_mesh->add_vertices();
//            auto pb_pos = pb_vert->mutable_position();
//            pb_pos->set_x(vert.position.x);
//            pb_pos->set_y(vert.position.y);
//            pb_pos->set_z(vert.position.z);
//            auto pb_norm = pb_vert->mutable_normal();
//            pb_norm->set_x(vert.normal.x);
//            pb_norm->set_y(vert.normal.y);
//            pb_norm->set_z(vert.normal.z);
//            auto pb_uv = pb_vert->mutable_uv();
//            pb_uv->set_x(vert.uv.x);
//            pb_uv->set_y(vert.uv.y);
//        }
//
//        for(const auto& idx: mesh->indices)
//            pb_mesh->add_indices(idx);
//
//        for(const auto& tex: mesh->textures)
//            pb_mesh->add_textures(tex->path);
//    }
//}
//
//void Model::protobuf_load(const std::string& path) {
//    GOOGLE_PROTOBUF_VERIFY_VERSION;
//
//    glp_util::Model model;
//
//    std::fstream input(path, std::ios::in | std::ios::binary);
//    if(!model.ParseFromIstream(&input)) {
//        glp_logv("failed to load model from protobuf %s", path.c_str());
//        return;
//    }
//
//    for(size_t i=0; i<model.meshes_size(); i++) {
//        auto pb_mesh = model.meshes(i);
//        std::vector<Vertex> vertices;
//        std::vector<unsigned int> idxs;
//        std::vector<Texture*> texs;
//
//        for(size_t j=0; j<pb_mesh.vertices_size(); j++) {
//            auto pb_vert = pb_mesh.vertices(j);
//            auto pb_pos = pb_vert.position();
//            auto pb_norm = pb_vert.normal();
//            auto pb_uv = pb_vert.uv();
//            vertices.emplace_back(
//                    glm::vec3(pb_pos.x(), pb_pos.y(), pb_pos.z()),
//                    glm::vec3(pb_norm.x(), pb_norm.y(), pb_norm.z()),
//                    glm::vec2(pb_uv.x(), pb_uv.y()));
//        }
//
//        for(size_t j=0; j<pb_mesh.indices_size(); j++) {
//            auto pb_idx = pb_mesh.indices(j);
//            idxs.push_back(pb_idx);
//        }
//
//        for(size_t j=0; j<pb_mesh.textures_size(); j++) {
//            auto tex = new Texture{pb_mesh.textures(j)};
//            texs.push_back(tex);
//        }
//
//        auto mesh = new Mesh(vertices, idxs, texs);
//        meshes.push_back(mesh);
//    }
//
//    google::protobuf::ShutdownProtobufLibrary();
//}
