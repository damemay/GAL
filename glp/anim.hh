#pragma once

#include "model.hh"

namespace Animation {

template <typename T>
struct Key {
    T value;
    float time;
};

struct Bone {
    std::vector<Key<glm::vec3>> positions;
    std::vector<Key<glm::quat>> rotations;
    std::vector<Key<glm::vec3>> scales;

    glm::mat4 local_transform;
    std::string name;
    int id;

    static Bone assimp_load(const std::string& name, int id, const aiNodeAnim* channel);
    void update(float time);

    int get_position_index(float time);
    int get_rotation_index(float time);
    int get_scale_index(float time);

    float get_scale_factor(float last, float next, float time);

    glm::mat4 interpolate_position(float time);
    glm::mat4 interpolate_rotation(float time);
    glm::mat4 interpolate_scale(float time);
};

struct Node {
    glm::mat4 transformation;
    std::string name;
    std::vector<Node> children;
};

class Animation {
    private:
        std::string name;
        float duration;
        float ticks_per_second;

        Node root_node;
        std::vector<Bone> bones;

        void read_assimp_hierarchy(Node& dest, const aiNode* src);

    public:
        inline const std::string& get_name() const { return name; }
        inline const float& get_duration() const { return duration; }
        inline const float& get_tps() const { return ticks_per_second; }
        inline const Node& get_root_node() const { return root_node; }
        Bone* find_bone(const std::string& name);

        Animation(const std::string& path, const Model& model, bool assimp);
        ~Animation() = default;

};

class Animator {
    private:
        std::vector<glm::mat4> bone_mat;
        Model* model;
        Animation* current_animation {nullptr};
        float current_time {0};
        float delta_time;

        void calc_bone_transform(const Node* node, glm::mat4 parent);

    public:
        inline const std::vector<glm::mat4> get_bone_matrices() const { return bone_mat; }

        void update(float dt);
        void play_animation(Animation* animation);
        
        Animator(Animation* animation, Model* model);
        ~Animator() = default;
};

}
