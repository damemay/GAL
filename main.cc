#include "glp/sdl.hh"
#include "glp/model.hh"
#include "glp/shader.hh"
#include "glp/anim.hh"

#include "glp/external/glm/glm.hpp"
#include "glp/external/glm/gtc/matrix_transform.hpp"
#include "glp/external/glm/gtc/type_ptr.hpp"

#include "glp/obj/camera.hh"
#include "glp/obj/player.hh"
#include "glp/obj/scenes.hh"
#include "glp/fonts.hh"
#include <sstream>

constexpr size_t WIDTH = 1280;
constexpr size_t HEIGHT = 720;
std::vector<Object::Static*> statics;

int main(int argc, char* argv[]) {
    Window sdl {"glp", WIDTH, HEIGHT};
    sdl.set_bg_color(glm::vec3(0.4, 0.4, 0.4));

    Object::Scene scene {"playground"};

    Object::Camera camera {glm::vec2(1280.0f, 720.0f), 90.0f};
    Object::PlayerFPP player {&camera, &sdl.events};
#ifndef __vita__
    player.use_mouse(true);
#else
    player.use_mouse(false);
#endif
    auto model = new Object::Static{"../res/models/plane/plane.model"};
    statics.push_back(model);
    auto man = new Object::Animated{"../res/models/anim/untitled.model", "../res/models/anim/untitled.anim", &sdl.dt};

    Font font {"../res/fonts/karla.png", WIDTH, HEIGHT};
    Text text {&font};
    text.update("enter space", 64, WIDTH/2, HEIGHT/2);

    model->translate(glm::vec3(0.0f, 0.0f, 0.0f));
    model->scale(glm::vec3(50.0f, 50.0f, 50.0f));

    man->translate(glm::vec3(0.0f, 50.0f, 0.0f));
    //man.rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    man->scale(glm::vec3(50.0f, 50.0f, 50.0f));


    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    while(sdl.is_running()) {
        sdl.loop_start();
        if(util::glerr()) glp_log("CLEAR");

        player.fpp_movement_keys();
        player.fpp_movement(sdl.dt);

        //model->render(camera);
        for(auto& e: statics) e->render(camera);
        glBindTexture(GL_TEXTURE_2D, 0);
        man->render(camera);

        text.render();

        util::glerr();

        sdl.loop_end();
    }

    return 0;
}
