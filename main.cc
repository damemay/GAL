#include "LinearMath/btVector3.h"
#include "external/glm/ext/matrix_transform.hpp"
#include "material.hh"
#include <sdl.hh>
#include <model.hh>
#include <obj/camera.hh>
#include <obj/player.hh>
#include <obj/light.hh>
#include <obj/collidable.hh>

constexpr int width = 1280;
constexpr int height = 720;

int main(int argc, char* argv[]) {
    glp::Window sdl {"glp-bullet", width, height};
    sdl.set_bg_color(glm::vec3(0.4,0.4,0.4));

    glp::Object::Camera camera {glm::vec2(width, height), 90.0f, 0.1f, 1000.0f};
    glp::Object::PlayerCollFPP player {25.0f, &camera, &sdl.events};
    player.use_mouse(true);

    glp::Shader shader {"../res/shaders/static.vert", "../res/shaders/pbr.frag"};

    shader.bind();
    glp::Object::Fog fog {&shader};
    glp::Object::Light light {glp::Object::LightType::DIRECTIONAL, &shader};
    light.set_color(glm::vec3(25,25,25));

    glp::Object::World world {};

    world.add_collidable(&player);

    glp::Model model {"../res/models/cube/cube.model"};
    glp::Object::CollRenderableModel object {&model, &shader, glp::ShadingType::PBR,
        1.0f, btVector3(1,10,0)};
    world.add_collidable(&object);

    glp::Object::CollRenderableModel object0 {&model, &shader, glp::ShadingType::PBR,
        1.0f, btVector3(0,-2,0)};
    world.add_collidable(&object0);

    glp::Object::CollRenderableModelSeparate sponza {"../res/models/test/test.model",
        &shader, glp::ShadingType::PBR, btVector3(0, -5, 0)};
    world.add_collidable(&sponza);
    //glp::Object::CollRenderableModel plane {"../res/models/plane/plane.model", &shader,
    //    glp::ShadingType::PBR, 0.0f, btVector3(0, -5, 0)};
    //world.add_collidable(&plane);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    while(sdl.is_running()) {
        sdl.loop_start();
        float dt = *sdl.get_dt_ptr();

        player.fpp_movement_keys();
        player.update();
        
        world.render(camera);
        world.update(dt);

        sdl.loop_end();
    }

    return EXIT_SUCCESS;
}
