#include "glp/sdl.hh"
#include "glp/model.hh"
#include "glp/shader.hh"
#include "glp/anim.hh"

#include "glp/external/glm/glm.hpp"
#include "glp/external/glm/gtc/matrix_transform.hpp"
#include "glp/external/glm/gtc/type_ptr.hpp"
#include <sstream>

int main(int argc, char* argv[]) {
    if(argc!=3) glp_log("[model path] [anim path]"), exit(1);
    Window sdl {"glp", 1280, 720};

    Shader skin_shader{"../res/shaders/skinned.vert", "../res/shaders/textured.frag"};
    //Shader static_shader{"../res/shaders/static.vert", "../res/shaders/textured.frag"};
    Model model (argv[1], &skin_shader);
    auto animation = Animation::Animation(argv[2], model);
    auto animator = Animation::Animator{&animation, &model};

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    auto cam_pos = glm::vec3(0.0f, 1.0f, 3.0f);
    auto cam_front = glm::vec3(0.0f, 0.0f, -1.0f);
    auto cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    float cam_yaw = -90.0f;
    bool moving = false;

    while(sdl.is_running()) {
        float spd = 100.0f * sdl.get_dt();
        while(SDL_PollEvent(&sdl.event) != 0) {
            if(sdl.event.type == SDL_QUIT) sdl.stop();
            if(sdl.event.type == SDL_KEYDOWN) {
                if(sdl.event.key.keysym.sym == SDLK_w) cam_pos += spd * cam_front;
                if(sdl.event.key.keysym.sym == SDLK_s) cam_pos -= spd * cam_front;
                if(sdl.event.key.keysym.sym == SDLK_a) cam_pos -= glm::normalize(glm::cross(cam_front, cam_up)) * spd;
                if(sdl.event.key.keysym.sym == SDLK_d) cam_pos += glm::normalize(glm::cross(cam_front, cam_up)) * spd;
                if(sdl.event.key.keysym.sym == SDLK_q) cam_yaw -= spd; cam_front = glm::normalize(glm::vec3(cos(glm::radians(cam_yaw)), 0.0f, sin(glm::radians(cam_yaw))));
                if(sdl.event.key.keysym.sym == SDLK_e) cam_yaw += spd; cam_front = glm::normalize(glm::vec3(cos(glm::radians(cam_yaw)), 0.0f, sin(glm::radians(cam_yaw))));
                if(sdl.event.key.keysym.sym == SDLK_z) cam_pos.y -= spd;
                if(sdl.event.key.keysym.sym == SDLK_x) cam_pos.y += spd;
            }
        }
        sdl.loop_start();
        if(util::glerr()) glp_log("CLEAR");

        auto view = glm::lookAt(cam_pos, cam_pos+cam_front, cam_up);
        auto projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(90.0f), 
                    (float)sdl.get_width()/(float)sdl.get_height(), 0.1f, 100000.0f);
        auto m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        auto mvp = projection * view * m;

        skin_shader.bind();
        skin_shader.set("mvp", mvp);
        animator.update(sdl.get_dt());
        //static_shader.bind();
        //static_shader.set("mvp", mvp);
        model.render();
        util::glerr();

        sdl.loop_end();
    }

    return 0;
}
