#pragma once

#include "camera.hh"
#include <SDL2/SDL.h>
#include <vector>

namespace Object {

class PlayerFPP {
    private:
        float move_speed {100.0f};
        glm::vec2 move_dir {0.0f, 0.0f};
        glm::vec2 rot_off {0.0f, 0.0f};

        bool firstmouse {true};
        bool mouse {false};

        Camera* camera{nullptr};
        std::vector<SDL_Event>* events{nullptr};

        void mouse_update(int x, int y);

    public:
        void fpp_movement_keys();
        void fpp_movement(float dt);

        inline void use_mouse(bool b) { mouse = b; }

        PlayerFPP(Camera* cam, std::vector<SDL_Event>* sdl) : camera{cam}, events{sdl} {}
        PlayerFPP(float speed, Camera* cam, std::vector<SDL_Event>* sdl) : move_speed{speed}, camera{cam}, events{sdl} {}
        ~PlayerFPP() = default;

};

}
