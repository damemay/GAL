#include <camera.hh>

namespace gal {
    void Basic_Camera::calculate() {
        if(pitch < -89.0f) pitch = -89.0f;
        if(pitch > 89.0f) pitch = 89.0f;
        front = glm::normalize(glm::vec3(
                    cos(glm::radians(yaw))*cos(glm::radians(pitch)),
                    sin(glm::radians(pitch)),
                    sin(glm::radians(yaw))*cos(glm::radians(pitch))));
    };
    
    glm::mat4 Basic_Camera::view() const {
        return glm::lookAt(position, position+front, up);
    }
    
    glm::mat4 Basic_Camera::view_projection() const {
        auto view = glm::lookAt(position, position+front, up);
        auto projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(fov), width/height, near, far);
        return projection * view;
    };
}
