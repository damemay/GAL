# glp

SDL2 + OpenGL 3.3 game engine/framework written in C++. Compatible with vitaGL.

## current features
- custom format for 3d models/animations with zstd compression
- conversion from standarized formats with assimp in separate util - [conv](utils/conv)
- ready pbr and phong lighting shaders
- 2d text rendering interface
- low and high level classes that range from just mesh rendering to building collision objects with bullet3
- fpp player movement and collisions with bullet3
- creating bullet3 scenes with lighting/fog options in separate util - [studio](utils/studio)

## example usage
[main.cc](main.cc) usually tests new features.

code snippet with setting up a scene with embedded shaders, plane ground and two rigidbody boxes:

```c++
#include <sdl.hh>
#include <obj/scene.hh>
#include <obj/builtin-shaders.hh>

constexpr int width = 1280;
constexpr int height = 720;

int main(int argc, char* argv[]) {
    glp::Window sdl {"glp-sample", width, height};
    sdl.set_bg_color(glm::vec3(0.4,0.4,0.4));

    auto [shader, shading_t] = glp::Object::make_static_pbr();

    glp::Object::Scene scene {width, height, &sdl.events, shader};
    scene.get_player()->use_mouse(true);

    scene.get_light().set_color(glm::vec3(25,25,25));

    scene.new_object(new glp::Object::CollRenderableModel{"../res/models/plane/plane.model", shader, shading_t,
            0.0f, btVector3(0, -5, 0)});

    glp::Model cube {"../res/models/cube/cube.model", &shader, shading_t};
    scene.new_object(new glp::Object::CollRenderableModel{&cube, shader, shading_t,
            1.0f, btVector3(0, -2, 0)});
    scene.new_object(new glp::Object::CollRenderableModel{&cube, shader, shading_t,
            1.0f, btVector3(1, 10, 0)});

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
```
