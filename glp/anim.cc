#include "anim.hh"
#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "external/glm/gtc/type_ptr.hpp"
#include "external/glm/gtx/quaternion.hpp"
#include "model.hh"

namespace Animation {

void Node::assimp_set_keys(const aiNodeAnim* channel) {
    for(size_t i=0; i<channel->mNumPositionKeys; i++)
        positions.emplace_back(
                glm::vec3(channel->mPositionKeys[i].mValue.x,
                    channel->mPositionKeys[i].mValue.y,
                    channel->mPositionKeys[i].mValue.z),
                channel->mPositionKeys[i].mTime);

    for(size_t i=0; i<channel->mNumRotationKeys; i++)
        rotations.emplace_back(
                glm::quat(channel->mRotationKeys[i].mValue.w,
                    channel->mRotationKeys[i].mValue.x,
                    channel->mRotationKeys[i].mValue.y,
                    channel->mRotationKeys[i].mValue.z),
                channel->mRotationKeys[i].mTime);

    for(size_t i=0; i<channel->mNumScalingKeys; i++)
        scales.emplace_back(
                glm::vec3(channel->mScalingKeys[i].mValue.x,
                    channel->mScalingKeys[i].mValue.y,
                    channel->mScalingKeys[i].mValue.z),
                channel->mScalingKeys[i].mTime);
}

void Node::update(float time) {
    auto t = interpolate_position(time);
    auto r = interpolate_rotation(time);
    auto s = interpolate_scale(time);
    local_transform = t*r*s;
}

int Node::get_position_index(float time) {
    for(size_t i=0; i<positions.size()-1; i++)
        if(time < positions[i+1].time) return i;
    return -1;
}

int Node::get_rotation_index(float time) {
    for(size_t i=0; i<rotations.size()-1; i++)
        if(time < rotations[i+1].time) return i;
    return -1;
}

int Node::get_scale_index(float time) {
    for(size_t i=0; i<scales.size()-1; i++)
        if(time < scales[i+1].time) return i;
    return -1;
}

float Node::get_scale_factor(float last, float next, float time) {
    float mid = time - last;
    float dif = next - last;
    return mid/dif;
}

glm::mat4 Node::interpolate_position(float time) {
    if(positions.size() <= 0) return glm::mat4(1.0f);
    int p = get_position_index(time);
    int n = p+1;
    float factor = get_scale_factor(positions[p].time, positions[n].time, time);
    glm::vec3 pos = glm::mix(positions[p].value, positions[n].value, factor);
    return glm::translate(glm::mat4(1.0f), pos);
}

glm::mat4 Node::interpolate_rotation(float time) {
    if(rotations.size() <= 0) return glm::mat4(1.0f);
    int p = get_rotation_index(time);
    int n = p+1;
    float factor = get_scale_factor(rotations[p].time, rotations[n].time, time);
    glm::quat rot = glm::slerp(rotations[p].value, rotations[n].value, factor);
    return glm::toMat4(rot);
}

glm::mat4 Node::interpolate_scale(float time) {
    if(scales.size() <= 0) return glm::mat4(1.0f);
    int p = get_scale_index(time);
    int n = p+1;
    float factor = get_scale_factor(scales[p].time, scales[n].time, time);
    glm::vec3 scale = glm::mix(scales[p].value, scales[n].value, factor);
    return glm::scale(glm::mat4(1.0f), scale);
}

Animation::Animation(const std::string& path, const Model& model, bool assimp) {
    if(assimp) {
        Assimp::Importer import;
        const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            glp_logv("assimp returned %s", import.GetErrorString());
            return;
        }

        if(!scene->HasAnimations() || scene->mNumAnimations < 0) {
            glp_log("wowzie! you just tried to load an animation that has no animations!");
            return;
        }

        auto anim = scene->mAnimations[0];
        name = anim->mName.C_Str();
        duration = anim->mDuration;
        ticks_per_second = anim->mTicksPerSecond;

        root_node = new Node;
        read_assimp_hierarchy(root_node, scene->mRootNode, model);

        for(size_t i=0; i<anim->mNumChannels; i++) {
            auto chan = anim->mChannels[i];
            std::string name = chan->mNodeName.C_Str();
            Node* node = find_node(root_node, name);
            node->assimp_set_keys(chan);
        }
    }
}

void Animation::clear_nodes(Node* parent) {
    for(auto& child: parent->children) {
        clear_nodes(child);
    }
    delete parent;
}

Animation::~Animation() {
    clear_nodes(root_node);
}

Node* Animation::find_node(Node* root, const std::string& name) {
    if(root->name == name) return root;
    for(size_t i=0; i<root->children.size(); i++) {
        Node* child = find_node(root->children[i], name);
        if(child) return child;
    }
    return nullptr;
}

void Animation::read_assimp_hierarchy(Node* dest, const aiNode* src, const Model& m) {
    dest->name = src->mName.C_Str();
    for(int i=0; i<m.get_bone_info().size(); i++) {
        auto bone = m.get_bone_info().at(i);
        if(bone.name == dest->name)
            dest->bone_index = i;
    }

    for(size_t i=0; i<src->mNumChildren; i++) {
        auto new_node = new Node;
        read_assimp_hierarchy(new_node, src->mChildren[i], m);
        dest->children.push_back(new_node);
    }
}

Animator::Animator(Animation* animation, Model* m) 
    : model{m}, current_animation{animation} {
    bone_mat.reserve(MAX_BONES);
    for(size_t i=0; i<MAX_BONES; i++)
        bone_mat.push_back(glm::mat4(1.0f));
}

void Animator::update(float dt) {
    for(size_t i=0; i<bone_mat.size(); i++)
        model->get_shader()->set("pose["+std::to_string(i)+"]", bone_mat[i]);
    delta_time = dt;
    if(current_animation) {
        current_time += current_animation->get_tps()*dt;
        current_time = std::fmod(current_time, current_animation->get_duration());
        calc_bone_transform(current_animation->get_root_node(), glm::mat4(1.0f));
    }
}

void Animator::play_animation(Animation* animation) {
    current_animation = animation;
    current_time = 0.0f;
}

void Animator::calc_bone_transform(Node* node, glm::mat4 parent) {
    auto global_transform = parent;

    node->update(current_time);

    if(node->bone_index > -1 && node->bone_index < model->get_bone_info().size()) {
        global_transform *= node->local_transform;
        bone_mat[node->bone_index] = global_transform * model->get_bone_info().at(node->bone_index).offset;
    }

    for(auto& child: node->children)
        calc_bone_transform(child, global_transform);
}

}
