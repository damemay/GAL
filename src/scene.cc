#include <scene.hh>
#include <gl.hh>

namespace gal {
    namespace scene {
        void Fog::set(const render::Material& material) {
            material.use();
            material.set(material.uniforms.at("fog.color"), color_);
            material.set(material.uniforms.at("fog.near"), near_);
            material.set(material.uniforms.at("fog.far"), far_);
            material.free();
        }

        void Fog::set_color(const render::Material& material, const glm::vec3& color) {
            color_ = color;
            material.use();
            material.set(material.uniforms.at("fog.color"), color_);
            material.free();
        }

        void Fog::set_near(const render::Material& material, const float near) {
            near_ = near;
            material.use();
            material.set(material.uniforms.at("fog.near"), near_);
            material.free();

        }

        void Fog::set_far(const render::Material& material, const float far) {
            far_ = far;
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
            color_ = color;
            material.use();
            material.set(material.uniforms.at("light.color"), color_);
            material.free();
        }

        void Directional_Light::set_position(const render::Material& material, const glm::vec3& position) {
            position_ = position;
            material.use();
            material.set(material.uniforms.at("light.position"), position_);
            material.free();
        }

        void Directional_Light::set_direction(const render::Material& material, const glm::vec3& direction) {
            direction_ = direction;
            material.use();
            material.set(material.uniforms.at("light.direction"), direction_);
            material.free();
        }
    };

    Scene::Scene(const glm::vec2& screen_dimensions) : screen_dimensions_{screen_dimensions} {}

    void Scene::set_fog(const glm::vec3& color, const float near, const float far) {
        for(const auto &[name, renderable]: renderables_) {
            for(auto& [prim, mat]: renderable->primitives) {
                fog_.set_color(mat, color);
                fog_.set_near(mat, near);
                fog_.set_far(mat, far);
            }
        }
    }

    void Scene::set_fog_color(const glm::vec3& color) {
        for(const auto &[name, renderable]: renderables_) {
            for(auto& [prim, mat]: renderable->primitives) {
                fog_.set_color(mat, color);
            }
        }
    }

    void Scene::set_fog_near(const float near) {
        for(const auto &[name, renderable]: renderables_) {
            for(auto& [prim, mat]: renderable->primitives) {
                fog_.set_near(mat, near);
            }
        }
    }

    void Scene::set_fog_far(const float far) {
        for(const auto &[name, renderable]: renderables_) {
            for(auto& [prim, mat]: renderable->primitives) {
                fog_.set_far(mat, far);
            }
        }
    }
    
    void Scene::set_light(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color) {
        for(const auto &[name, renderable]: renderables_) {
            for(auto& [prim, mat]: renderable->primitives) {
                light_.set_position(mat, position);
                light_.set_direction(mat, direction);
                light_.set_color(mat, color);
            }
        }
    }

    void Scene::set_light_position(const glm::vec3& position) {
        for(const auto &[name, renderable]: renderables_) {
            for(auto& [prim, mat]: renderable->primitives) {
                light_.set_position(mat, position);
            }
        }
    }

    void Scene::set_light_direction(const glm::vec3& direction) {
        for(const auto &[name, renderable]: renderables_) {
            for(auto& [prim, mat]: renderable->primitives) {
                light_.set_direction(mat, direction);
            }
        }
    }

    void Scene::set_light_color(const glm::vec3& color) {
        for(const auto &[name, renderable]: renderables_) {
            for(auto& [prim, mat]: renderable->primitives) {
                light_.set_color(mat, color);
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
