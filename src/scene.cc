#include <scene.hh>
#include <gl.hh>
#include <util.hh>
#include <shader_generator.hh>
#include <utility>

namespace gal {
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
            environment.light()->set(deref_material);
            environment.fog()->set(deref_material);
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
        environment.fog()->color = color;
        environment.fog()->near = near;
        environment.fog()->far = far;
    }

    void Scene::set_fog_color(const glm::vec3& color) {
        environment.fog()->color = color;
    }

    void Scene::set_fog_near(const float near) {
        environment.fog()->near = near;
    }

    void Scene::set_fog_far(const float far) {
        environment.fog()->far = far;
    }
    
    void Scene::set_light(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color) {
        environment.light()->position = position;
        environment.light()->direction = direction;
        environment.light()->color = color;
    }

    void Scene::set_light_position(const glm::vec3& position) {
        environment.light()->position = position;
    }

    void Scene::set_light_direction(const glm::vec3& direction) {
        environment.light()->direction = direction;
    }

    void Scene::set_light_color(const glm::vec3& color) {
        environment.light()->color = color;
    }

    void Scene::render_depth_map(const render::Shader& shader) {
        glm::mat4 pos{1.0f};
        shader.use();
        shader.set("vp", current_camera_->view_projection());
        shader.set("camera_position", current_camera_->position);
        for(auto& [name, renderable]: renderables_) {
            for(auto& [primitive, mat]: renderable->primitives) {
                shader.set("model", pos);
                glBindVertexArray(primitive.vao);
                glDrawElements(GL_TRIANGLES, primitive.indices.size(), GL_UNSIGNED_INT, nullptr);
                glBindVertexArray(0);
            }
        }
    }

    uint8_t Scene::bind_textures(const render::Material& material) {
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
        return count;
    }

    void Scene::render() {
        glm::mat4 pos{1.0f};
        for(auto &[shader, pairs]: shaders_materials_) {
            glUseProgram(shader);
            render::Shader r_shader {shader};
            r_shader.set("vp", current_camera_->view_projection());
            r_shader.set("camera_position", current_camera_->position);
            r_shader.set("light_space_mat", environment.light_space_matrix());
            for(auto& pair: pairs) {
                auto primitive = pair.first;
                auto material = pair.second;
                material.set(material.uniforms.at("model"), pos);
                auto count = bind_textures(material);
                glActiveTexture(GL_TEXTURE0+count);
                glBindTexture(GL_TEXTURE_2D, environment.depth_map_texture());
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

        environment.use_depth_map();
        render_depth_map(environment.depth_map_shader());
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, screen_dimensions_.x, screen_dimensions_.y);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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
