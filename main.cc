#include "external/glm/ext/matrix_transform.hpp"
#include "material.hh"
#include <sdl.hh>
#include <model.hh>
#include <obj/camera.hh>
#include <obj/player.hh>
#include <obj/light.hh>
#include <obj/collidable.hh>
#include <obj/scene.hh>

constexpr int width = 1280;
constexpr int height = 720;

int main(int argc, char* argv[]) {
    glp::Window sdl {"glp-bullet", width, height};
    sdl.set_bg_color(glm::vec3(0.4,0.4,0.4));

    glp::Shader shader {"../res/shaders/static.vert", "../res/shaders/pbr.frag"};
    glp::Object::Scene scene {width, height, &sdl.events, &shader};

    scene.get_player()->use_mouse(true);
    scene.get_light().set_color(glm::vec3(25,25,25));

    scene.new_object(new glp::Object::CollRenderableModel{"../res/models/plane/plane.model", &shader,
            glp::ShadingType::PBR, 0.0f, btVector3(0, -5, 0)});

    glp::Model cube {"../res/models/cube/cube.model"};
    scene.new_object(new glp::Object::CollRenderableModel{&cube, &shader,
            glp::ShadingType::PBR, 1.0f, btVector3(0, -2, 0)});
    scene.new_object(new glp::Object::CollRenderableModel{&cube, &shader,
            glp::ShadingType::PBR, 1.0f, btVector3(1, 10, 0)});

    SDL_SetRelativeMouseMode(SDL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    while(sdl.is_running()) {
        sdl.loop_start();
        float dt = *sdl.get_dt_ptr();

        scene.update(dt);

        sdl.loop_end();
    }

    return EXIT_SUCCESS;
}
