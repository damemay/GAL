#pragma once

#include <scene.hh>

namespace glp {
    struct Basic_FPP_Controller: scene::Controller {
        void mouse_update(int x, int y);
        void handle_sdl_events(const std::vector<SDL_Event>& sdl_events); 
        void delta_time_update(float delta_time);

        Basic_FPP_Controller(float move_speed, float mouse_sensitivity);
        ~Basic_FPP_Controller() = default;

        protected:
            glm::vec2 move_dir_ {};
            glm::vec2 rot_off_ {};

            bool first_mouse_ {true};

            float move_speed_ {};
            float mouse_sensitivity_ {0.1f};
    };
}
