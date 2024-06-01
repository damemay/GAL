#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sdl.hh>
#include <scene.hh>
#include <material.hh>
#include <prim.hh>

namespace glp {
    struct GLTF_Render_Model: scene::Renderable {
        GLTF_Render_Model(const std::string& path);
        ~GLTF_Render_Model();
    };
}

