#include <scene.hh>
#include <gl.hh>
#include <util.hh>
#include <shader_generator.hh>
#include <utility>

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
        for(const auto &[name, renderable]: renderables_) {
            for(auto &[prim, mat]: renderable->primitives) {
                if(auto it = shaders_materials_.find(mat.shader); it == shaders_materials_.end()) {
                    shaders_materials_.insert({mat.shader, {std::make_pair(prim, mat)}});
                } else {
                    it->second.push_back(std::make_pair(prim, mat));
                }
            }
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

    void Scene::render() {
        glm::mat4 pos{1.0f};
        for(auto it = shaders_materials_.begin(); it != shaders_materials_.end(); it++) {
            glUseProgram(it->first);
            for(auto& pair: it->second) {
                auto primitive = pair.first;
                auto material = pair.second;
                material.set(material.uniforms.at("vp"), current_camera_->view_projection());
                material.set(material.uniforms.at("camera_position"), current_camera_->position);
                material.set(material.uniforms.at("model"), pos);
                {
                    uint8_t count = 0;
                    if(material.albedo_id >= 0) {
                        glActiveTexture(GL_TEXTURE0+count);
                        material.set(material.uniforms.at("material.albedo_tex"), count);
                        glBindTexture(GL_TEXTURE_2D, material.textures.at(material.albedo_id));
                        count++;
                    }
                    if(material.metallic_id >= 0) {
                        glActiveTexture(GL_TEXTURE0+count);
                        material.set(material.uniforms.at("material.metallic_tex"), count);
                        glBindTexture(GL_TEXTURE_2D, material.textures.at(material.metallic_id));
                        count++;
                    }
                    if(material.roughness_id >= 0) {
                        glActiveTexture(GL_TEXTURE0+count);
                        material.set(material.uniforms.at("material.roughness_tex"), count);
                        glBindTexture(GL_TEXTURE_2D, material.textures.at(material.roughness_id));
                        count++;
                    }
                    if(material.occlusion_id >= 0) {
                        glActiveTexture(GL_TEXTURE0+count);
                        material.set(material.uniforms.at("material.occlusion_tex"), count);
                        glBindTexture(GL_TEXTURE_2D, material.textures.at(material.occlusion_id));
                        count++;
                    }
                    if(material.normal_id >= 0) {
                        glActiveTexture(GL_TEXTURE0+count);
                        material.set(material.uniforms.at("material.normal_tex"), count);
                        glBindTexture(GL_TEXTURE_2D, material.textures.at(material.normal_id));
                        count++;
                    }
                }
                glBindVertexArray(primitive.vao);
                glDrawElements(GL_TRIANGLES, primitive.indices.size(), GL_UNSIGNED_INT, nullptr);
                glBindVertexArray(0);
            }
        }
    }

    void Scene::loop(float delta_time, const std::vector<SDL_Event>& sdl_events) {
        glClearColor(background_color_.x, background_color_.y, background_color_.z, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if(current_controller_) {
            current_controller_->handle_sdl_events(sdl_events);
            current_controller_->delta_time_update(delta_time);
        }

        if(callback_) callback_();

        render();
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
