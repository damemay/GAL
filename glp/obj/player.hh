#pragma once

#include <camera.hh>
#include <obj/collidable.hh>
#include <SDL2/SDL.h>
#include <vector>

namespace glp {

namespace Object {

class Player {
    public:
        virtual ~Player() {}
};

class PlayerFPP : public Player {
    protected:
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

        inline float get_speed() { return move_speed; }

        PlayerFPP(Camera* cam, std::vector<SDL_Event>* sdl) : camera{cam}, events{sdl} {}
        PlayerFPP(float speed, Camera* cam, std::vector<SDL_Event>* sdl) : move_speed{speed}, camera{cam}, events{sdl} {}
        ~PlayerFPP() = default;

};

class PlayerCollFPP : public PlayerFPP, public Collidable {
    private:
        using PlayerFPP::fpp_movement;
        glm::vec3 cam_off {0.0f, 2.0f, 0.0f};

    public:
        void update();

        inline void set_camera_offset(glm::vec3 off) { cam_off = off; }

        PlayerCollFPP(float speed, Camera* cam, std::vector<SDL_Event>* sdl);
        ~PlayerCollFPP() = default;
};

}

}
