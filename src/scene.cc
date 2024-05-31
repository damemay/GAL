#include <scene.hh>
#include <gl.hh>

namespace glp {
    Scene::Scene(const glm::vec2& screen_dimensions) : screen_dimensions_{screen_dimensions} {

    }

    void Scene::loop(float delta_time, const std::vector<SDL_Event>& sdl_events) {
        glClearColor(background_color_.x, background_color_.y, background_color_.z, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(current_controller_) {
            current_controller_->handle_sdl_events(sdl_events);
            current_controller_->delta_time_update(delta_time);
        }

        for(const auto &[name, renderable]: renderables_) {
            renderable->render();
        }
    }

    void Scene::set_controller(const std::string& name) {
        current_controller_ = controllers_.at(name).get();
        if(current_camera_)
            current_controller_->camera = current_camera_;
    }

    void Scene::set_camera(const std::string& name) {
        current_camera_ = cameras_.at(name).get();
        if(current_controller_)
            current_controller_->camera = current_camera_;
    }
}
