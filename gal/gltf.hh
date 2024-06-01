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

            Model(const std::string& path);

            private:
                void load(const std::string& path);
                void setup_primitives();
        };
    }
}
