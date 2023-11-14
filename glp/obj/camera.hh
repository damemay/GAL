#pragma once

#include "../external/glm/glm.hpp"
#include "../external/glm/gtc/matrix_transform.hpp"
#include "../external/glm/gtc/type_ptr.hpp"

namespace Object {

class Camera {
    private:
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

    public:
        inline void calculate() {
            if(pitch < -89.0f) pitch = -89.0f;
            if(pitch > 89.0f) pitch = 89.0f;
            front = glm::normalize(glm::vec3(
                        cos(glm::radians(yaw))*cos(glm::radians(pitch)),
                        sin(glm::radians(pitch)),
                        sin(glm::radians(yaw))*cos(glm::radians(pitch))));
        };

        inline glm::mat4 view_projection() {
            auto view = glm::lookAt(position, position+front, up);
            auto projection = glm::mat4(1.0f);
            projection = glm::perspective(glm::radians(fov), 
                        width/height, near, far);
            return projection * view;
        };

        inline void yaw_change(float add) { yaw += add; }
        inline void pitch_change(float add) { pitch += add; }
        inline void position_change(glm::vec3 add) { position += add; }

        inline void set_position(glm::vec3 pos) { position = pos; }
        inline void set_yaw(float val) { yaw = val; calculate(); }
        inline void set_pitch(float val) { pitch = val; calculate(); }
        inline void set_near(float val) { near = val; }
        inline void set_far(float val) { far = val; }
        inline void set_fov(float val) { fov = val; }

        inline glm::vec3 get_up() { return up; }
        inline glm::vec3 get_front() { return front; }
        inline glm::vec3 get_position() { return position; }
        inline glm::vec2 get_dimensions() { return glm::vec2(width, height); }

        Camera() = default;
        Camera(float x, float y, float z) : position{glm::vec3(x, y, z)} {}
        Camera(float width_, float height_) : width{width_}, height{height_} {}
        Camera(glm::vec2 dim, float fov_) : width{dim.x}, height{dim.y}, fov{fov_} {}
        Camera(glm::vec2 dim, float fov_, float near_, float far_) : width{dim.x}, height{dim.y}, fov{fov_}, near{near_}, far{far_} {}
        Camera(glm::vec3 pos, glm::vec2 dim) : position{pos}, width{dim.x}, height{dim.y} {}
        Camera(glm::vec3 pos, glm::vec2 dim, float fov_) : position{pos}, width{dim.x}, height{dim.y}, fov{fov_} {}
        Camera(glm::vec3 pos, glm::vec2 dim, float fov_, float near_, float far_) : position{pos}, width{dim.x}, height{dim.y}, fov{fov_}, near{near_}, far{far_} {}

        ~Camera() = default;
};

}
