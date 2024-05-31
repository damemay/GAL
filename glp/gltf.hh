#pragma once

#include <tiny_gltf.h>
#include <model.hh>

namespace glp {
    namespace gltf {
        tinygltf::Model load_model(const std::string& path);
        std::vector<render::Primitive> setup_primitives(tinygltf::Model& model);
    }
}
