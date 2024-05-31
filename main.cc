#include <glm/ext/matrix_transform.hpp>
#include <material.hh>
#include <obj/renderable.hh>
#include <sdl.hh>
#include <model.hh>
#include <obj/camera.hh>
#include <obj/player.hh>
#include <obj/light.hh>
#include <obj/collidable.hh>
#include <obj/scene.hh>
#include <obj/builtin-shaders.hh>

constexpr int width = 1280;
constexpr int height = 720;

int main(int argc, char* argv[]) {
    glp::Window sdl {"glp-bullet", width, height};
    sdl.set_bg_color(glm::vec3(0.4,0.4,0.4));

    auto [shader, shading_t] = glp::Object::make_static_phong();

    glp::Object::PhysicsScene scene {"../res/scenes/plane/planes.scene", width, height, &sdl.events, shader, shading_t};
    scene.set_debug(true);

    scene.get_player()->use_mouse(true);

    scene.get_light().set_color(glm::vec3(25,25,25));

    [[maybe_unused]] auto [anim_shader, _] = glp::Object::make_skinned_pbr();

    std::vector<glp::Object::Animated*> anims;

    anims.push_back(new glp::Object::Animated {"../res/models/anim/untitled.model", "../res/models/anim/untitled.anim",
        sdl.get_dt_ptr(), anim_shader, shading_t});

    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    while(sdl.is_running()) {
        sdl.loop_start();
        float dt = *sdl.get_dt_ptr();

        scene.update(dt);
        for(auto& anim: anims)
            anim->render(scene.get_camera());
        //anim.render(scene.get_camera());

        sdl.loop_end();
    }

    return EXIT_SUCCESS;
}
