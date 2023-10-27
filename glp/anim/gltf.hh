#pragma once

#include <cstring>
#include <cstdio>
#include <vector>
#include <string>

#include "external/cgltf.h"
#include "external/glm/gtc/type_ptr.hpp"

#include "utils.h"
#include "transform.hh"
#include "meshes.hh"
#include "anim.hh"

namespace gltf {
    
    cgltf_data* load(const char* path);
    void free(cgltf_data* data);
    transform::_t get_loc_transform(cgltf_node& n);

    int get_node_index(cgltf_node* target, cgltf_node* all_nodes, unsigned int nodes_count);
    Pose load_rest_pose(cgltf_data* data);

    std::vector<std::string> load_joint_names(cgltf_data* data);
    void get_scalar_values(std::vector<float>& out, unsigned int comp_count, const cgltf_accessor& acc);

    template<typename T, int N>
    void track_from_channel(anim::Track<T, N>& result, const cgltf_animation_channel& chan) {
        cgltf_animation_sampler& sampler = *chan.sampler;

        anim::Interpolation interp = anim::Interpolation::constant;
        if(sampler.interpolation == cgltf_interpolation_type_linear)
            interp = anim::Interpolation::linear;
        else if(sampler.interpolation == cgltf_interpolation_type_cubic_spline)
            interp = anim::Interpolation::cubic;

        bool is_sampler_cub = (interp == anim::Interpolation::cubic);
        result.set_interpolation(interp);

        std::vector<float> time;
        get_scalar_values(time, 1, *sampler.input);

        std::vector<float> val;
        get_scalar_values(val, N, *sampler.output);

        unsigned int frames = sampler.input->count;
        unsigned int comp_count = val.size() / time.size();
        result.resize(frames);

        for(unsigned int i=0; i<frames; ++i) {
            int base_idx = i*comp_count;
            anim::Frame<N>& frame = result[i];
            int offset = 0;
            frame.time = time[i];

            for(int comp=0; comp<N; ++comp)
                frame.in[comp] = is_sampler_cub ? val[base_idx+offset++] : 0.0f;
            for(int comp=0; comp<N; ++comp)
                frame.value[comp] = val[base_idx+offset++];
            for(int comp=0; comp<N; ++comp)
                frame.out[comp] = is_sampler_cub ? val[base_idx+offset++] : 0.0f;
        }
    }

    std::vector<anim::Clip> load_animation_clips(cgltf_data* data);
    Pose load_bind_pose(cgltf_data* data);

    Skeleton load_skeleton(cgltf_data* data);
    void mesh_from_attr(Mesh& out, cgltf_attribute& attr, cgltf_skin* skin, cgltf_node* nodes, unsigned int node_count);
    std::vector<Mesh> load_meshes(cgltf_data* data);

}
