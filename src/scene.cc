#include <scene.hh>
#include <gl.hh>

namespace glp {
    Scene::Scene(const glm::vec2& screen_dimensions) : screen_dimensions_{screen_dimensions} {

    }

    void Scene::loop(float delta_time, const std::vector<SDL_Event>& sdl_events) {
        glClearColor(background_color_.x, background_color_.y, background_color_.z, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}
