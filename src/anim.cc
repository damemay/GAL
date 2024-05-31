#include <anim.hh>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <model.hh>
#include <utils.hh>
#include <sstream>
#include <fstream>

namespace glp {

namespace Animation {

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

Animation::Animation(const std::string& path, const Model& model) {
    auto file = util::read_file(path);
    auto decompressed = util::decompress(file);
    std::stringstream s;
    s << decompressed;
    deserialize_data(model, s);
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

Animator::Animator(Animation* animation, Model* m) 
    : model{m}, current_animation{animation} {
    bone_mat.reserve(MAX_BONES);
    for(size_t i=0; i<MAX_BONES; i++)
        bone_mat.push_back(glm::mat4(1.0f));
}

void Animator::update(float dt) {
    model->get_shader()->set("pose", bone_mat);
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

void Animation::serialize_nodes(Node* parent, std::stringstream& s) {
    s << "node " << parent->name << ' ';
    s << "pk " << parent->positions.size() << ' ';
    for(size_t i=0; i<parent->positions.size(); i++)
        s << "p " << parent->positions[i].value.x << ' '
            << parent->positions[i].value.y << ' '
            << parent->positions[i].value.z << ' '
            << parent->positions[i].time << ' ';
    s << "rk " << parent->rotations.size() << ' ';
    for(size_t i=0; i<parent->rotations.size(); i++)
        s << "r " << parent->rotations[i].value.w << ' '
            << parent->rotations[i].value.x << ' '
            << parent->rotations[i].value.y << ' '
            << parent->rotations[i].value.z << ' '
            << parent->rotations[i].time << ' ';
    s << "sk " << parent->scales.size() << ' ';
    for(size_t i=0; i<parent->scales.size(); i++)
        s << "s " << parent->scales[i].value.x << ' '
            << parent->scales[i].value.y << ' '
            << parent->scales[i].value.z << ' '
            << parent->scales[i].time << ' ';

    s << "cdn " << parent->children.size() << ' ';
    for(size_t i=0; i<parent->children.size(); i++)
        serialize_nodes(parent->children[i], s);
    s << "cdnend ";
}

std::stringstream Animation::serialize_data() {
    std::stringstream s;
    if(name.empty()) name = "anim";
    s << name << ' ' << duration << ' ' << ticks_per_second << ' ';
    serialize_nodes(root_node, s);
    return s;
}

void Animation::deserialize_nodes(Node*& parent, const Model& m, std::stringstream& s) {
    std::string name;
    size_t count;
    s >> name; 
    assert(name == "node");
    s >> name;
    parent->name = name;
    glp_logv("%s", parent->name.c_str());
    for(int i=0; i<m.get_bone_info().size(); i++) {
        auto bone = m.get_bone_info().at(i);
        if(bone.name == parent->name)
            parent->bone_index = i;
    }
    s >> name; assert(name == "pk");
    s >> count;
    parent->positions.resize(count);
    for(size_t i=0; i<parent->positions.size(); i++) {
        s >> name; assert(name == "p");
        s >> parent->positions[i].value.x
            >> parent->positions[i].value.y 
            >> parent->positions[i].value.z
            >> parent->positions[i].time;
    }
    s >> name; assert(name == "rk");
    s >> count;
    parent->rotations.resize(count);
    for(size_t i=0; i<parent->rotations.size(); i++) {
        s >> name; assert(name == "r");
        s >> parent->rotations[i].value.w
            >> parent->rotations[i].value.x 
            >> parent->rotations[i].value.y 
            >> parent->rotations[i].value.z
            >> parent->rotations[i].time;
    }
    s >> name; assert(name == "sk");
    s >> count;
    parent->scales.resize(count);
    for(size_t i=0; i<parent->scales.size(); i++) {
        s >> name; assert(name == "s");
        s >> parent->scales[i].value.x
            >> parent->scales[i].value.y 
            >> parent->scales[i].value.z
            >> parent->scales[i].time;
    }
    s >> name; assert(name == "cdn");
    s >> count;
    //parent->children.resize(count);
    for(size_t i=0; i<count; i++) {
        auto node = new Node;
        deserialize_nodes(node, m, s);
        parent->children.push_back(node);
    }
    s >> name; assert(name == "cdnend");
}

void Animation::deserialize_data(const Model& m, std::stringstream& s) {
    s >> name >> duration >> ticks_per_second;
    root_node = new Node;
    deserialize_nodes(root_node, m, s);
}

}

}
