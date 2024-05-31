#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

namespace glp {
    class Scene {
        glm::vec2 screen_dimensions_;
        glm::vec3 background_color_;

        public:
            Scene(const glm::vec2& screen_dimensions);

            void loop(float delta_time, const std::vector<SDL_Event>& sdl_events);

            inline void set_background_color(const glm::vec3& color) { background_color_ = color; }
    };
}
