#pragma once

#include <tiny_gltf.h>
#include <model.hh>
#include <material.hh>
#include <unordered_map>

namespace gal {
    namespace gltf {
        struct Model {
            tinygltf::Model tinygltf_model;
            std::unordered_map<render::Primitive, render::Material> primitives;
            std::unordered_map<uint8_t, GLuint> loaded_textures;

            Model(const std::string& path);

            private:
                tinygltf::Sampler default_sampler;
                
                void load(const std::string& path);
                void setup_primitives();
                void setup_textures(int gltf_texture_id, int8_t& material_texture_id, std::map<uint8_t, GLuint>& material_textures);
        };
    }
}
