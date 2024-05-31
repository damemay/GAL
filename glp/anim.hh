#pragma once

#include <model.hh>

namespace glp {

namespace Animation {

template <typename T>
struct Key {
    T value;
    float time;
};

struct Node {
    std::string name;
    int bone_index {-1};
    glm::mat4 local_transform {glm::mat4(1.0f)};
    std::vector<Node*> children;

    std::vector<Key<glm::vec3>> positions;
    std::vector<Key<glm::quat>> rotations;
    std::vector<Key<glm::vec3>> scales;

    void update(float time);

    int get_position_index(float time);
    int get_rotation_index(float time);
    int get_scale_index(float time);

    float get_scale_factor(float last, float next, float time);

    glm::mat4 interpolate_position(float time);
    glm::mat4 interpolate_rotation(float time);
    glm::mat4 interpolate_scale(float time);
};

class Animation {
    private:
        std::string name;
        float duration;
        float ticks_per_second;

        Node* root_node;

        void clear_nodes(Node* parent);
        void serialize_nodes(Node* parent, std::stringstream& s);
        void deserialize_nodes(Node*& parent, const Model& m, std::stringstream& s);
        void deserialize_data(const Model& m, std::stringstream& s);

    public:
        inline const std::string& get_name() const { return name; }
        inline const float& get_duration() const { return duration; }
        inline const float& get_tps() const { return ticks_per_second; }
        inline Node* get_root_node() const { return root_node; }
        Node* find_node(Node* root, const std::string& name);

        std::stringstream serialize_data();

        Animation(const std::string& path, const Model& model);
        Animation() {};
        ~Animation();

};

class Animator {
    private:
        std::vector<glm::mat4> bone_mat;
        Model* model;
        Animation* current_animation {nullptr};
        float current_time {0};
        float delta_time;

        void calc_bone_transform(Node* node, glm::mat4 parent);

    public:
        inline const std::vector<glm::mat4> get_bone_matrices() const { return bone_mat; }

        inline Animation* get_animation() { return current_animation; }

        void update(float dt);
        void play_animation(Animation* animation);
        
        Animator(Animation* animation, Model* model);
        Animator() {};
        ~Animator() = default;
};

}

}
