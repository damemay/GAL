#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <scene.hh>

namespace glp {
    struct Basic_Camera : scene::Camera {
        void calculate();
        glm::mat4 view() const;
        glm::mat4 view_projection() const;
    
        Basic_Camera() = default;
        ~Basic_Camera() = default;
    };
}
