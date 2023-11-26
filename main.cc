#include "external/glm/ext/matrix_transform.hpp"
#include "material.hh"
#include <sdl.hh>
#include <model.hh>
#include <obj/camera.hh>
#include <obj/player.hh>
#include <obj/light.hh>
#include <obj/collidable.hh>
#include <obj/scene.hh>
#include <obj/builtin-shaders.hh>

#ifndef __vita__
constexpr int width = 1280;
constexpr int height = 720;
#else
constexpr int width = 960;
constexpr int height = 544;
#endif

int main(int argc, char* argv[]) {
    glp::Window sdl {"glp-bullet", width, height};
    sdl.set_bg_color(glm::vec3(0.4,0.4,0.4));

#ifndef __vita__
    auto [shader, shading_t] = glp::Object::make_static_phong();
#else
    auto [shader, shading_t] = glp::Object::make_static_phong();
#endif

    glp::Object::PhysicsScene scene {"../res/scenes/plane/planes.scene", width, height, &sdl.events, shader, shading_t};
    scene.set_debug(true);

#ifndef __vita__
    scene.get_player()->use_mouse(true);
#endif

    scene.get_light().set_color(glm::vec3(25,25,25));

#ifndef __vita__
    [[maybe_unused]] auto [anim_shader, _] = glp::Object::make_skinned_pbr();
#else
    [[maybe_unused]] auto [anim_shader, _] = glp::Object::make_skinned_phong();
#endif

    glp::Object::Animated anim {"../res/models/anim/untitled.model", "../res/models/anim/untitled.anim",
        sdl.get_dt_ptr(), anim_shader, shading_t};

    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    while(sdl.is_running()) {
        sdl.loop_start();
        float dt = *sdl.get_dt_ptr();

        scene.update(dt);
        anim.render(scene.get_camera());

        sdl.loop_end();
    }

    return EXIT_SUCCESS;
}
