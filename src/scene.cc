#include <scene.hh>
#include <gl.hh>

namespace glp {
    namespace scene {
        void Fog::set(const render::Material& material) {
            material.use();
            material.set(material.uniforms.at("fog.color"), color_);
            material.set(material.uniforms.at("fog.near"), near_);
            material.set(material.uniforms.at("fog.far"), far_);
            material.free();
        }

        void Fog::set_color(const render::Material& material, const glm::vec3& color) {
            material.use();
            material.set(material.uniforms.at("fog.color"), color_);
            material.free();
        }

        void Fog::set_near(const render::Material& material, const float near) {
            material.use();
            material.set(material.uniforms.at("fog.near"), near_);
            material.free();

        }

        void Fog::set_far(const render::Material& material, const float far) {
            material.use();
            material.set(material.uniforms.at("fog.far"), far_);
            material.free();
        }

        void Directional_Light::set(const render::Material& material) {
            material.use();
            material.set(material.uniforms.at("light.position"), position_);
            material.set(material.uniforms.at("light.direction"), direction_);
            material.set(material.uniforms.at("light.color"), color_);
            material.free();
        }

        void Directional_Light::set_color(const render::Material& material, const glm::vec3& color) {
            material.use();
            material.set(material.uniforms.at("light.color"), color_);
            material.free();
        }

        void Directional_Light::set_position(const render::Material& material, const glm::vec3& position) {
            material.use();
            material.set(material.uniforms.at("light.position"), position_);
            material.free();
        }

        void Directional_Light::set_direction(const render::Material& material, const glm::vec3& direction) {
            material.use();
            material.set(material.uniforms.at("light.direction"), direction_);
            material.free();
        }
    };

    Scene::Scene(const glm::vec2& screen_dimensions) : screen_dimensions_{screen_dimensions} {}

    void Scene::loop(float delta_time, const std::vector<SDL_Event>& sdl_events) {
        glClearColor(background_color_.x, background_color_.y, background_color_.z, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(current_controller_) {
            current_controller_->handle_sdl_events(sdl_events);
            current_controller_->delta_time_update(delta_time);
        }

        if(callback_) callback_();

        for(const auto &[name, renderable]: renderables_) {
            for(auto& [prim, mat]: renderable->primitives) {
                glm::mat4 pos{1.0f};
                mat.use();
                mat.set(mat.uniforms.at("vp"), current_camera_->view_projection());
                mat.set(mat.uniforms.at("camera_position"), current_camera_->position);
                mat.set(mat.uniforms.at("model"), pos);
                glBindVertexArray(prim.vao);
                glDrawElements(GL_TRIANGLES, prim.indices.size(), GL_UNSIGNED_INT, nullptr);
                glBindVertexArray(0);
                mat.free();
            }
        }
    }

    void Scene::set_controller(const std::string& name) {
        current_controller_ = get_controller(name);
        if(current_camera_) current_controller_->camera = current_camera_;
    }

    void Scene::set_camera(const std::string& name) {
        current_camera_ = get_camera(name);
        if(current_controller_) current_controller_->camera = current_camera_;
    }
}
