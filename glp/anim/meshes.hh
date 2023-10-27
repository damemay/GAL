#pragma once

#include <vector>
#include <cstring>
#include <string>

#include "transform.hh"
#include "attribute.hh"
#include "indexbuffer.hh"

class Pose {
    protected:
        std::vector<transform::_t> joints;
        std::vector<int> parents;

    public:
        Pose() = default;
        inline Pose(unsigned int joints_count) { resize(joints_count); }
        inline Pose(const Pose& p) { *this = p; }
        Pose& operator=(const Pose& p);

        inline void resize(unsigned int size) {
            parents.resize(size);
            joints.resize(size);
        }

        inline unsigned int size() { return joints.size(); }
        inline int get_parent(unsigned int idx) { return parents[idx]; }
        inline void set_parent(unsigned int idx, int parent) { parents[idx] = parent; }
        inline transform::_t get_loc_transform(unsigned int idx) { return joints[idx]; }
        inline void set_loc_transform(unsigned int idx, const transform::_t& trans) { joints[idx] = trans; }
        transform::_t get_glob_transform(unsigned int idx);
        inline transform::_t operator[](unsigned int idx) { return get_glob_transform(idx); }
        void get_mat_palette(std::vector<glm::mat4>& out);
        bool operator==(const Pose& p);
        inline bool operator!=(const Pose& p) { return !(*this == p); }
};

class Skeleton {
    protected:
        Pose rest_pose;
        Pose bind_pose;
        std::vector<glm::mat4> inv_bind_pose;
        std::vector<std::string> joint_names;

        void update_inverse_bind_pose();

    public:
        void set(const Pose& rest, const Pose& bind, const std::vector<std::string>& names);
        inline Skeleton(const Pose& rest, const Pose& bind, const std::vector<std::string>& names) { set(rest, bind, names); }

        inline Pose& get_bind_pose() { return bind_pose; }
        inline Pose& get_rest_pose() { return rest_pose; }

        inline std::vector<glm::mat4>& get_inv_bind_pose() { return inv_bind_pose; }
        inline std::vector<std::string>& get_joint_names() { return joint_names; }
        inline std::string& get_joint_name(unsigned int idx) { return joint_names[idx]; }
};

class Mesh {
    protected:
        std::vector<glm::vec3> position;
        std::vector<glm::vec3> normal;
        std::vector<glm::vec2> tex_coord;
        std::vector<glm::vec4> weights;
        std::vector<glm::ivec4> influences;
        std::vector<unsigned int> indices;

        Attribute<glm::vec3>* pos_attrib;
        Attribute<glm::vec3>* norm_attrib;
        Attribute<glm::vec2>* uv_attrib;
        Attribute<glm::vec4>* weight_attrib;
        Attribute<glm::ivec4>* influence_attrib;
        IndexBuffer* index_buffer;

    public:
        Mesh();
        Mesh(const Mesh& m);
        Mesh& operator=(const Mesh& m);
        ~Mesh();

        inline std::vector<glm::vec3>& get_position() { return position; }
        inline std::vector<glm::vec3>& get_normal() { return normal; }
        inline std::vector<glm::vec2>& get_tex_coord() { return tex_coord; }
        inline std::vector<glm::vec4>& get_weights() { return weights; }
        inline std::vector<glm::ivec4>& get_influences() { return influences; }
        inline std::vector<unsigned int>& get_indices() { return indices; }

        void update_gl_buffers();

        void render();
        void render_instanced(unsigned int count);

        void bind(int pos, int norm, int tex, int weight, int inf);
        void unbind(int pos, int norm, int tex, int weight, int inf);

};
