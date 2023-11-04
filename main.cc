#include "glp/sdl.hh"
#include "glp/model.hh"
#include "glp/shader.hh"
#include "glp/anim.hh"

#include "glp/external/glm/glm.hpp"
#include "glp/external/glm/gtc/matrix_transform.hpp"
#include "glp/external/glm/gtc/type_ptr.hpp"

#include "glp/obj/camera.hh"
#include "glp/obj/player.hh"
#include "glp/obj/placables.hh"
#include <sstream>

constexpr size_t WIDTH = 1280;
constexpr size_t HEIGHT = 720;

int main(int argc, char* argv[]) {
    Window sdl {"glp", WIDTH, HEIGHT};
    Object::Camera camera {glm::vec2(1280.0f, 720.0f), 90.0f};
    Object::Player player {&camera, &sdl.events};
    Object::Static model {"../res/models/sponza/sponza.model"};
    Object::Animated man {"../res/models/simplerig/simplerig.model", "../res/models/simplerig/simplerig.anim"};

    man.translate(glm::vec3(0.0f, 0.0f, 0.0f));
    man.rotate(-90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    man.scale(glm::vec3(50.0f, 50.0f, 50.0f));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    while(sdl.is_running()) {
        sdl.loop_start();
        if(util::glerr()) glp_log("CLEAR");

        player.fpp_movement_keys();
        player.fpp_movement(sdl.get_dt());

        model.translate(glm::vec3(0.0f, 0.0f, 0.0f));
        model.render(camera);
        man.render(camera, sdl.get_dt());

        util::glerr();

        sdl.loop_end();
    }

    return 0;
}
