#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <scene.hh>

namespace glp {
    struct Basic_Camera : scene::Camera {
        glm::vec3 position      {0.0f, 1.0f, 3.0f};
        glm::vec3 front         {0.0f, 0.0f, -1.0f};
        glm::vec3 up            {0.0f, 1.0f, 0.0f};

        float yaw               {-90.0f};
        float pitch             {0.0f};
    
        float near              {0.1f};
        float far               {100.0f};
    
        float fov               {60.0f};
        float width             {960.0};
        float height            {544.0};
    
        void calculate();
        glm::mat4 view() const;
        glm::mat4 view_projection() const;
    
        Basic_Camera() = default;
        ~Basic_Camera() = default;
    };
}
