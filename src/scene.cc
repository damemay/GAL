#include <scene.hh>
#include <gl.hh>
#include <util.hh>
#include <shader_generator.hh>

namespace gal {
    namespace scene {
        void Fog::set(const render::Material& material) {
            material.set(material.uniforms.at("fog.color"), color);
            material.set(material.uniforms.at("fog.near"), near);
            material.set(material.uniforms.at("fog.far"), far);
        }

        void Directional_Light::set(const render::Material& material) {
            material.set(material.uniforms.at("light.position"), position);
            material.set(material.uniforms.at("light.direction"), direction);
            material.set(material.uniforms.at("light.color"), color);
        }
    }

    Scene::Scene(const glm::vec2& screen_dimensions) : screen_dimensions_{screen_dimensions} {}

    void Scene::init() {
#ifdef GAL_DEBUG
        util::print(std::format("generating shaders for scene..."));
#endif
        std::vector<render::Material*> materials;
        for(const auto &[name, renderable]: renderables_)
            for(auto &[prim, mat]: renderable->primitives)
                materials.push_back(&mat);
        auto generator = render::Shader_Generator(materials);
        for(const auto& material: materials) {
            material->setup_uniforms();
            auto deref_material = *material;
            light_.set(deref_material);
            fog_.set(deref_material);
        }
    }

    void Scene::set_fog(const glm::vec3& color, const float near, const float far) {
        fog_.color = color;
        fog_.near = near;
        fog_.far = far;
    }

    void Scene::set_fog_color(const glm::vec3& color) {
        fog_.color = color;
    }

    void Scene::set_fog_near(const float near) {
        fog_.near = near;
    }

    void Scene::set_fog_far(const float far) {
        fog_.far = far;
    }
    
    void Scene::set_light(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color) {
        light_.position = position;
        light_.direction = direction;
        light_.color = color;
    }

    void Scene::set_light_position(const glm::vec3& position) {
        light_.position = position;
    }

    void Scene::set_light_direction(const glm::vec3& direction) {
        light_.direction = direction;
    }

    void Scene::set_light_color(const glm::vec3& color) {
        light_.color = color;
    }

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
                {
                    uint8_t count = 0;
                    if(mat.albedo_id >= 0) {
                        glActiveTexture(GL_TEXTURE0+count);
                        mat.set(mat.uniforms.at("material.albedo_tex"), count);
                        glBindTexture(GL_TEXTURE_2D, mat.textures.at(mat.albedo_id));
                        count++;
                    }
                    if(mat.metallic_id >= 0) {
                        glActiveTexture(GL_TEXTURE0+count);
                        mat.set(mat.uniforms.at("material.metallic_tex"), count);
                        glBindTexture(GL_TEXTURE_2D, mat.textures.at(mat.metallic_id));
                        count++;
                    }
                    if(mat.roughness_id >= 0) {
                        glActiveTexture(GL_TEXTURE0+count);
                        mat.set(mat.uniforms.at("material.roughness_tex"), count);
                        glBindTexture(GL_TEXTURE_2D, mat.textures.at(mat.roughness_id));
                        count++;
                    }
                    if(mat.occlusion_id >= 0) {
                        glActiveTexture(GL_TEXTURE0+count);
                        mat.set(mat.uniforms.at("material.occlusion_tex"), count);
                        glBindTexture(GL_TEXTURE_2D, mat.textures.at(mat.occlusion_id));
                        count++;
                    }
                    if(mat.normal_id >= 0) {
                        glActiveTexture(GL_TEXTURE0+count);
                        mat.set(mat.uniforms.at("material.normal_tex"), count);
                        glBindTexture(GL_TEXTURE_2D, mat.textures.at(mat.normal_id));
                        count++;
                    }
                }
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
