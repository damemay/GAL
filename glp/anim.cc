#include "anim.hh"
#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "external/glm/gtc/type_ptr.hpp"
#include "model.hh"

namespace Animation {

Bone Bone::assimp_load(const std::string& name, int id, const aiNodeAnim* channel) {
    Bone bone = {
        .local_transform = glm::mat4(1.0f),
        .name = name,
        .id = id,
    };

    for(size_t i=0; i<channel->mNumPositionKeys; i++)
        bone.positions.emplace_back(
                glm::vec3(channel->mPositionKeys[i].mValue.x,
                    channel->mPositionKeys[i].mValue.y,
                    channel->mPositionKeys[i].mValue.z),
                channel->mPositionKeys[i].mTime);

    for(size_t i=0; i<channel->mNumRotationKeys; i++)
        bone.rotations.emplace_back(
                glm::quat(channel->mRotationKeys[i].mValue.x,
                    channel->mRotationKeys[i].mValue.y,
                    channel->mRotationKeys[i].mValue.z,
                    channel->mRotationKeys[i].mValue.w),
                channel->mRotationKeys[i].mTime);

    for(size_t i=0; i<channel->mNumScalingKeys; i++)
        bone.scales.emplace_back(
                glm::vec3(channel->mScalingKeys[i].mValue.x,
                    channel->mScalingKeys[i].mValue.y,
                    channel->mScalingKeys[i].mValue.z),
                channel->mScalingKeys[i].mTime);

    return bone;
}

void Bone::update(float time) {
    auto t = interpolate_position(time);
    auto r = interpolate_rotation(time);
    auto s = interpolate_scale(time);
    local_transform = t*r*s;
}

int Bone::get_position_index(float time) {
    for(size_t i=0; i<positions.size()-1; i++)
        if(time < positions[i+1].time) return i;
    return -1;
}

int Bone::get_rotation_index(float time) {
    for(size_t i=0; i<rotations.size()-1; i++)
        if(time < rotations[i+1].time) return i;
    return -1;
}

int Bone::get_scale_index(float time) {
    for(size_t i=0; i<scales.size()-1; i++)
        if(time < scales[i+1].time) return i;
    return -1;
}

float Bone::get_scale_factor(float last, float next, float time) {
    float mid = time - last;
    float dif = next - last;
    return mid/dif;
}

glm::mat4 Bone::interpolate_position(float time) {
    int p = get_position_index(time);
    int n = p+1;
    float factor = get_scale_factor(positions[p].time, positions[n].time, time);
    glm::vec3 pos = glm::mix(positions[p].value, positions[n].value, factor);
    return glm::translate(glm::mat4(1.0f), pos);
}

glm::mat4 Bone::interpolate_rotation(float time) {
    int p = get_rotation_index(time);
    int n = p+1;
    float factor = get_scale_factor(rotations[p].time, rotations[n].time, time);
    glm::quat rot = glm::slerp(rotations[p].value, rotations[n].value, factor);
    return glm::mat4(rot);
}

glm::mat4 Bone::interpolate_scale(float time) {
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

        read_assimp_hierarchy(root_node, scene->mRootNode);

        for(size_t i=0; i<anim->mNumChannels; i++) {
            auto chan = anim->mChannels[i];
            std::string name = chan->mNodeName.C_Str();
            for(size_t j=0; j<model.get_bone_info().size(); j++) {
                auto bone_info = model.get_bone_info().at(j);
                if(name == bone_info.name) {
                    bones.push_back(Bone::assimp_load(chan->mNodeName.C_Str(),
                        bone_info.id, chan));
                }
            }
        }
    }
}

Bone* Animation::find_bone(const std::string& name) {
    for(auto& bone: bones)
        if(bone.name == name)
            return &bone;
    return nullptr;
}

void Animation::read_assimp_hierarchy(Node& dest, const aiNode* src) {
    dest.name = src->mName.C_Str();
    dest.transformation = glm::mat4(src->mTransformation.a1,
            src->mTransformation.b1, src->mTransformation.c1,
            src->mTransformation.d1, src->mTransformation.a2,
            src->mTransformation.b2, src->mTransformation.c2, 
            src->mTransformation.d2, src->mTransformation.a3,
            src->mTransformation.b3, src->mTransformation.c3,
            src->mTransformation.d3, src->mTransformation.a4,
            src->mTransformation.b4, src->mTransformation.c4,
            src->mTransformation.d4);

    for(size_t i=0; i<src->mNumChildren; i++) {
        Node new_node;
        read_assimp_hierarchy(new_node, src->mChildren[i]);
        dest.children.push_back(new_node);
    }
}

Animator::Animator(Animation* animation, Model* m) : model{m}, current_animation{animation} {
    bone_mat.reserve(MAX_BONES);
    for(size_t i=0; i<MAX_BONES; i++)
        bone_mat.push_back(glm::mat4(1.0f));
}

void Animator::update(float dt) {
    delta_time = dt;
    if(current_animation) {
        current_time += current_animation->get_tps()*dt;
        current_time = std::fmod(current_time, current_animation->get_duration());
        calc_bone_transform(&current_animation->get_root_node(), glm::mat4(1.0f));
    }
}

void Animator::play_animation(Animation* animation) {
    current_animation = animation;
    current_time = 0.0f;
}

void Animator::calc_bone_transform(const Node* node, glm::mat4 parent) {
    std::string name = node->name;
    //glm::mat4 transform = node->transformation;
    auto transform = glm::mat4(1.0f);

    Bone* bone = current_animation->find_bone(name);
    if(bone) {
        bone->update(current_time);
        transform = bone->local_transform;
    }

    glm::mat4 global_transform = parent * transform;

    for(size_t i=0; i<model->get_bone_info().size(); i++) {
        auto bone = model->get_bone_info().at(i);
        if(bone.name == name) {
            bone_mat[bone.id] = global_transform * bone.offset;
            break;
        }
    }

    for(const auto& child: node->children)
        calc_bone_transform(&child, global_transform);
}

}
