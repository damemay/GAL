#include <controller.hh>

namespace gal {
    Basic_FPP_Controller::Basic_FPP_Controller(float move_speed, float mouse_sensitivity) : move_speed_{move_speed}, mouse_sensitivity_{mouse_sensitivity} {
        SDL_SetRelativeMouseMode(SDL_TRUE);
    }

    void Basic_FPP_Controller::mouse_update(int x, int y) {
        if(first_mouse_) {
            x = camera->width/2;
            y = camera->height/2;
            first_mouse_ = false;
        }
        rot_off_.x = x;
        rot_off_.y = y;

        rot_off_.x*=mouse_sensitivity_;
        rot_off_.y*=mouse_sensitivity_;
        camera->yaw+=rot_off_.x;
        camera->pitch-=rot_off_.y;
        camera->calculate();
    }

    void Basic_FPP_Controller::handle_sdl_events(const std::vector<SDL_Event>& sdl_events) {
        for(auto& event: sdl_events) {
            if(event.type == SDL_KEYDOWN) {
                if(event.key.keysym.sym == SDLK_w) move_dir_.y = 1.0f;
                if(event.key.keysym.sym == SDLK_s) move_dir_.y = -1.0f;
                if(event.key.keysym.sym == SDLK_a) move_dir_.x = -1.0f;
                if(event.key.keysym.sym == SDLK_d) move_dir_.x = 1.0f;
            } else if(event.type == SDL_KEYUP) {
                if(event.key.keysym.sym == SDLK_w) move_dir_.y = 0.0f;
                if(event.key.keysym.sym == SDLK_s) move_dir_.y = 0.0f;
                if(event.key.keysym.sym == SDLK_a) move_dir_.x = 0.0f;
                if(event.key.keysym.sym == SDLK_d) move_dir_.x = 0.0f;
            } else if(event.type == SDL_MOUSEMOTION) {
                if(camera) mouse_update(event.motion.xrel, event.motion.yrel);
            }
        }
    }

    void Basic_FPP_Controller::delta_time_update(float delta_time) {
        if(camera) {
            camera->position+=(move_speed_*delta_time*camera->front)*move_dir_.y;
            camera->position+=(glm::normalize(glm::cross(camera->front, camera->up))*move_speed_*delta_time)*move_dir_.x;
        }
    }
}
