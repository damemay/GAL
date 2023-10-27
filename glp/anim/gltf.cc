#include "gltf.hh"
#include "external/glm/gtc/type_ptr.hpp"

namespace gltf {

    cgltf_data* load(const char* path) {
        cgltf_options opt;
        memset(&opt, 0, sizeof(cgltf_options));

        cgltf_data* data = nullptr;
        cgltf_result res;

        if((res = cgltf_parse_file(&opt, path, &data)) != cgltf_result_success) {
            glp_logv("could not load gltf: %s", path);
            return nullptr;
        }

        if((res = cgltf_load_buffers(&opt, data, path)) != cgltf_result_success) {
            glp_logv("could not load gltf: %s", path);
            return nullptr;
        }

        if((res = cgltf_validate(data)) != cgltf_result_success) {
            glp_logv("invalid gltf: %s", path);
            return nullptr;
        }

        return data;
    }

    void free(cgltf_data* data) {
        if(data) cgltf_free(data);
    }

    transform::_t get_loc_transform(cgltf_node& n) {
        transform::_t res;
        if(n.has_matrix) {
            glm::mat4 mat = glm::make_mat4(&n.matrix[0]);
            res = transform::from_mat4(mat);
        }
        if(n.has_translation)
            res.position = glm::vec3(n.translation[0], n.translation[1], n.translation[2]);
        if(n.has_rotation)
            res.rotation = glm::quat(n.rotation[0], n.rotation[1], n.rotation[2], n.rotation[3]);
        if(n.has_scale)
            res.scale = glm::vec3(n.scale[0], n.scale[1], n.scale[2]);
        return res;
    }

    int get_node_index(cgltf_node* target, cgltf_node* all_nodes, unsigned int nodes_count) {
        if(!target) return -1;
        for(unsigned int i=0; i<nodes_count; ++i)
            if(target == &all_nodes[i]) return (int)i;
        return -1;
    }

    Pose load_rest_pose(cgltf_data* data) {
        unsigned int bones = data->nodes_count;
        Pose result {bones};
        for(unsigned int i=0; i<bones; ++i) {
            cgltf_node* node = &(data->nodes[i]);
            transform::_t t = get_loc_transform(data->nodes[i]);
            result.set_loc_transform(i, t);
            int parent = get_node_index(node->parent, data->nodes, bones);
            result.set_parent(i, parent);
        }
        return result;
    }

    std::vector<std::string> load_joint_names(cgltf_data* data) {
        unsigned int bones = data->nodes_count;
        std::vector<std::string> result(bones, "not set");
        for(unsigned int i=0; i<bones; ++i) {
            cgltf_node* node = &(data->nodes[i]);
            if(!node->name) result[i] = "empty node";
            else result[i] = node->name;
        }
        return result;
    }

    void get_scalar_values(std::vector<float>& out, unsigned int comp_count, const cgltf_accessor& acc) {
        out.resize(acc.count*comp_count);
        for(cgltf_size i=0; i<acc.count; ++i)
            cgltf_accessor_read_float(&acc, i, &out[i*comp_count], comp_count);
    }

    std::vector<anim::Clip> load_animation_clips(cgltf_data* data) {
        unsigned int clips = data->animations_count;
        unsigned int nodes = data->nodes_count;
        std::vector<anim::Clip> result;
        result.resize(clips);

        for(unsigned int i=0; i<clips; ++i) {
            result[i].set_name(data->animations[i].name);
            unsigned int channels = data->animations[i].channels_count;
            for(unsigned int j=0; j<channels; ++j) {
                cgltf_animation_channel& chan = data->animations[i].channels[j];
                cgltf_node* target = chan.target_node;
                int node_id = get_node_index(target, data->nodes, nodes);

                if(chan.target_path == cgltf_animation_path_type_translation) {
                    anim::vector_track& track = result[i][node_id].get_position_track();
                    track_from_channel<glm::vec3, 3>(track, chan);
                } else if(chan.target_path == cgltf_animation_path_type_scale) {
                    anim::vector_track& track = result[i][node_id].get_scale_track();
                    track_from_channel<glm::vec3, 3>(track, chan);
                } else if(chan.target_path == cgltf_animation_path_type_rotation) {
                    anim::quat_track& track = result[i][node_id].get_rotation_track();
                    track_from_channel<glm::quat, 4>(track, chan);
                }
            }
            result[i].recalc_duration();
        }

        return result;
    }

    Pose load_bind_pose(cgltf_data* data) {
        Pose rest = load_rest_pose(data);
        unsigned int bones = rest.size();

        std::vector<transform::_t> w_bindpose(bones);
        for(unsigned int i=0; i<bones; ++i)
            w_bindpose[i] = rest.get_glob_transform(i);

        unsigned int skins = data->skins_count;
        for(unsigned int i=0; i<skins; ++i) {
            cgltf_skin* skin = &(data->skins[i]);
            std::vector<float> invbind_acc;
            get_scalar_values(invbind_acc, 16, *skin->inverse_bind_matrices);
            unsigned int joints = skin->joints_count;
            for(int j=0; j<joints; ++j) {
                float* matrix = &(invbind_acc[j*16]);
                glm::mat4 invbind_mat = glm::make_mat4(matrix);
                glm::mat4 bind_mat = glm::inverse(invbind_mat);
                auto bind_t = transform::from_mat4(bind_mat);
                cgltf_node* joint_n = skin->joints[j];
                int joint_idx = get_node_index(joint_n, data->nodes, bones);
            }
        }

        Pose bind = rest;
        for(unsigned int i=0; i<bones; ++i) {
            auto cur = w_bindpose[i];
            int p = bind.get_parent(i);
            if(p>=0) {
                auto parent = w_bindpose[p];
                cur = transform::combine(transform::inverse(parent), cur);
            }
            bind.set_loc_transform(i, cur);
        }

        return bind;
    }

    Skeleton load_skeleton(cgltf_data* data) {
        return Skeleton(
                load_rest_pose(data),
                load_bind_pose(data),
                load_joint_names(data));
    }

    void mesh_from_attr(Mesh& out, cgltf_attribute& attr, cgltf_skin* skin, cgltf_node* nodes, unsigned int node_count) {
        cgltf_accessor& acc = *attr.data;
        cgltf_attribute_type attrib_t = attr.type;

        unsigned int comp_count = 0;
        if(acc.type == cgltf_type_vec2) comp_count = 2;
        else if(acc.type == cgltf_type_vec3) comp_count = 3;
        else if(acc.type == cgltf_type_vec4) comp_count = 4;

        std::vector<float> values;
        get_scalar_values(values, comp_count, acc);

        unsigned int acc_count = acc.count;
        std::vector<glm::vec3>& pos = out.get_position();
        std::vector<glm::vec3>& norm = out.get_normal();
        std::vector<glm::vec2>& tex = out.get_tex_coord();
        std::vector<glm::ivec4>& inf = out.get_influences();
        std::vector<glm::vec4>& wei = out.get_weights();

        for(unsigned int i=0; i<acc_count; ++i) {
            int index = i*comp_count;
            switch(attrib_t) {
                case cgltf_attribute_type_position:
                    pos.push_back(glm::vec3(values[index+0], values[index+1], values[index+2]));
                    break;
                case cgltf_attribute_type_texcoord:
                    tex.push_back(glm::vec2(values[index+0], values[index+1]));
                    break;
                case cgltf_attribute_type_weights:
                    wei.push_back(glm::vec4(values[index+0], values[index+1], values[index+2], values[index+3]));
                    break;
                case cgltf_attribute_type_normal:
                    {
                    auto normal = glm::vec3(values[index+0], values[index+1], values[index+2]);
                    if(glm::length2(normal) < 0.000001f) normal = glm::vec3(0, 1, 0);
                    norm.push_back(glm::normalize(normal));
                    }
                    break;
                case cgltf_attribute_type_joints:
                    {
                    auto joints = glm::ivec4(
                            (int)(values[index+0]+0.5f),
                            (int)(values[index+1]+0.5f),
                            (int)(values[index+2]+0.5f),
                            (int)(values[index+3]+0.5f));
                    joints.x = get_node_index(skin->joints[joints.x], nodes, node_count);
                    joints.y = get_node_index(skin->joints[joints.y], nodes, node_count);
                    joints.z = get_node_index(skin->joints[joints.z], nodes, node_count);
                    joints.w = get_node_index(skin->joints[joints.w], nodes, node_count);
                    joints.x = std::max(0, joints.x);
                    joints.y = std::max(0, joints.y);
                    joints.z = std::max(0, joints.z);
                    joints.w = std::max(0, joints.w);
                    }
                    break;
                default:break;
            }
        }
    }

    std::vector<Mesh> load_meshes(cgltf_data* data) {
        std::vector<Mesh> res;
        cgltf_node* nodes = data->nodes;
        unsigned int n_count = data->nodes_count;

        for(unsigned int i=0; i<n_count; ++i) {
            cgltf_node* node = &nodes[i];
            if(!node->mesh || !node->skin) continue;
            int p_count = node->mesh->primitives_count;

            for(int j=0; j<p_count; ++j) {
                res.push_back(Mesh());
                Mesh& mesh = res[res.size()-1];
                cgltf_primitive* prim = &node->mesh->primitives[i];
                int ac = prim->attributes_count;
                for(unsigned int k=0; k<ac; ++k) {
                    cgltf_attribute* attr = &prim->attributes[k];
                    mesh_from_attr(mesh, *attr, node->skin, nodes, n_count);
                }
                if(prim->indices) {
                    int ic = prim->indices->count;
                    std::vector<unsigned int>& indices = mesh.get_indices();
                    indices.resize(ic);
                    for(unsigned int k=0; k<ic; ++k)
                        indices[k] = cgltf_accessor_read_index(prim->indices, k);
                }
                mesh.update_gl_buffers();
            }
        }
        return res;
    }

}
