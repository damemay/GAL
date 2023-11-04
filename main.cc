#include "glp/sdl.hh"
#include "glp/model.hh"
#include "glp/shader.hh"
#include "glp/anim.hh"

#include "glp/external/glm/glm.hpp"
#include "glp/external/glm/gtc/matrix_transform.hpp"
#include "glp/external/glm/gtc/type_ptr.hpp"
#include <sstream>

constexpr size_t WIDTH = 1280;
constexpr size_t HEIGHT = 720;

auto cam_pos = glm::vec3(0.0f, 1.0f, 3.0f);
auto cam_front = glm::vec3(0.0f, 0.0f, -1.0f);
auto cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
float cam_yaw = -90.0f;
float cam_pitch = 0.0f;
bool moving = false;
float x = 0, z = 0;

bool firstmouse = true;

void calc_rot() {
    if(cam_pitch < -89.0f) cam_pitch = -89.0f;

    cam_front = glm::normalize(glm::vec3(
                cos(glm::radians(cam_yaw))*cos(glm::radians(cam_pitch)),
                sin(glm::radians(cam_pitch)),
                sin(glm::radians(cam_yaw))*cos(glm::radians(cam_pitch))));
}

void mouse(int x, int y) {
    if(firstmouse) {
        x = WIDTH/2, y = HEIGHT/2;
        firstmouse = false;
    }
    float xoff = x;
    float yoff = y;

    float sens = 0.1f;
    xoff*=sens;
    yoff*=sens;

    cam_yaw+=xoff;
    cam_pitch-=yoff;
    calc_rot();
}

int main(int argc, char* argv[]) {
    Window sdl {"glp", WIDTH, HEIGHT};

    //Shader skin_shader{"../res/shaders/skinned.vert", "../res/shaders/textured.frag"};
#ifndef __vita__
    Shader static_shader{"../res/shaders/static.vert", "../res/shaders/textured.frag"};
#else 
    Shader static_shader{"../res/shaders/vita_static.vert", "../res/shaders/vita_textured.frag"};
#endif

    Model model ("../res/models/sponza/sponza.model", &static_shader);
    //auto animation = Animation::Animation(argv[2], model);
    //auto animator = Animation::Animator{&animation, &model};

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    while(sdl.is_running()) {
        float spd = 100.0f * sdl.get_dt();
        while(SDL_PollEvent(&sdl.event) != 0) {
            if(sdl.event.type == SDL_QUIT) sdl.stop();
            if(sdl.event.type == SDL_KEYDOWN) {
                if(sdl.event.key.keysym.sym == SDLK_w) moving = true, z = 1;
                if(sdl.event.key.keysym.sym == SDLK_s) moving = true, z = -1;
                if(sdl.event.key.keysym.sym == SDLK_a) moving = true, x = -1;
                if(sdl.event.key.keysym.sym == SDLK_d) moving = true, x = 1;
            }
            if(sdl.event.type == SDL_KEYUP) {
                if(sdl.event.key.keysym.sym == SDLK_w) moving = false, z = 0;
                if(sdl.event.key.keysym.sym == SDLK_s) moving = false, z = 0;
                if(sdl.event.key.keysym.sym == SDLK_a) moving = false, x = 0;
                if(sdl.event.key.keysym.sym == SDLK_d) moving = false, x = 0;
            }
            if(sdl.event.type == SDL_MOUSEMOTION)
                mouse(sdl.event.motion.xrel, sdl.event.motion.yrel);

            if(sdl.event.type == SDL_CONTROLLERAXISMOTION) {
                if(sdl.event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX) {
                    if(sdl.event.caxis.value < -8000) moving = true, x = -1;
                    if(sdl.event.caxis.value > 8000) moving = true, x = 1;
                }
                if(sdl.event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) {
                    if(sdl.event.caxis.value < -8000) moving = true, z = 1;
                    if(sdl.event.caxis.value > 8000) moving = true, z = -1;
                }
                if(sdl.event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX) {
                    if(sdl.event.caxis.value < -8000 || sdl.event.caxis.value > 8000) {
                        cam_yaw+= sdl.event.caxis.value*0.1f;
                        calc_rot();
                    }
                }
                if(sdl.event.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY) {
                    if(sdl.event.caxis.value < -8000 || sdl.event.caxis.value > 8000) {
                        cam_pitch-= sdl.event.caxis.value*0.1f;
                        calc_rot();
                    }
                }
            }
        }
        sdl.loop_start();
        if(util::glerr()) glp_log("CLEAR");

        if(moving) {
            cam_pos += (spd*cam_front)*z;
            cam_pos += (glm::normalize(glm::cross(cam_front, cam_up))*spd)*x;
        }

        auto view = glm::lookAt(cam_pos, cam_pos+cam_front, cam_up);
        auto projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(90.0f), 
                    (float)sdl.get_width()/(float)sdl.get_height(), 0.1f, 100000.0f);
        auto m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        auto mvp = projection * view * m;

        //skin_shader.bind();
        //skin_shader.set("mvp", mvp);
        //animator.update(sdl.get_dt());
        static_shader.bind();
        static_shader.set("mvp", mvp);
        model.render();
        util::glerr();

        sdl.loop_end();
    }

    return 0;
}
