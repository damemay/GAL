#include "player.hh"

namespace Object {

void Player::mouse_update(int x, int y) {
    if(firstmouse) {
        x = camera->get_dimensions().x/2,
          y = camera->get_dimensions().y/2;
        firstmouse = false;
    }
    float xoff = x;
    float yoff = y;

    float sens = 0.1f;
    xoff*=sens;
    yoff*=sens;

    camera->yaw_change(xoff);
    camera->pitch_change(-yoff);
    camera->calculate();
}

void Player::fpp_movement(float dt) {
    camera->position_change((move_speed*dt*camera->get_front())*move_dir.y);
    camera->position_change((glm::normalize(glm::cross(camera->get_front(), camera->get_up()))*move_speed*dt)*move_dir.x);
}

void Player::fpp_movement_keys() {
    for(auto& event: *events) {
        if(event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym == SDLK_w) move_dir.y = 1.0f;
            if(event.key.keysym.sym == SDLK_s) move_dir.y = -1.0f;
            if(event.key.keysym.sym == SDLK_a) move_dir.x = -1.0f;
            if(event.key.keysym.sym == SDLK_d) move_dir.x = 1.0f;
        } else if(event.type == SDL_KEYUP) {
            if(event.key.keysym.sym == SDLK_w) move_dir.y = 0.0f;
            if(event.key.keysym.sym == SDLK_s) move_dir.y = 0.0f;
            if(event.key.keysym.sym == SDLK_a) move_dir.x = 0.0f;
            if(event.key.keysym.sym == SDLK_d) move_dir.x = 0.0f;
        } else if(event.type == SDL_MOUSEMOTION) {
            mouse_update(event.motion.xrel, event.motion.yrel);
        } else if(event.type == SDL_CONTROLLERAXISMOTION) {
            if(event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                if(event.caxis.value < -8000) move_dir.x = -1.0f;
                else if(event.caxis.value > 8000) move_dir.x = 1.0f;
                else if(event.caxis.value > -8000 && event.caxis.value < 8000) move_dir.x = 0.0f;
            }
            if(event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                if(event.caxis.value < -8000) move_dir.y = 1.0f;
                else if(event.caxis.value > 8000) move_dir.y = -1.0f;
                else if(event.caxis.value > -8000 && event.caxis.value < 8000) move_dir.y = 0.0f;
            }
            if(event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
                if(event.caxis.value < -8000 || event.caxis.value > 8000) {
                    camera->yaw_change(event.caxis.value*0.01f);
                    camera->calculate();
                }
            }
            if(event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
                if(event.caxis.value < -8000 || event.caxis.value > 8000) {
                    camera->pitch_change(event.caxis.value*-0.01f);
                    camera->calculate();
                }
            }
        }
    }
}

}
