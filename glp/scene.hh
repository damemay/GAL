#pragma once

#include <map>
#include <type_traits>
#include <vector>
#include <memory>
#include <functional>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <prim.hh>
#include <material.hh>

namespace glp {
    namespace scene {
        struct Renderable {
            std::unordered_map<render::Primitive, render::Material> primitives;

            virtual ~Renderable() {};
        };

        struct Camera {
            glm::vec3 position      {0.0f, 1.0f, 3.0f};
            glm::vec3 front         {0.0f, 0.0f, -1.0f};
            glm::vec3 up            {0.0f, 1.0f, 0.0f};

            float yaw               {-90.0f};
            float pitch             {0.0f};
    
            float near              {0.1f};
            float far               {100.0f};
    
            float fov               {60.0f};
            float width             {};
            float height            {};
    
            void setup_dimensions(const glm::vec2& dimensions) {
                width = dimensions.x;
                height = dimensions.y;
            }
            virtual void calculate() = 0;
            virtual glm::mat4 view() const = 0;
            virtual glm::mat4 view_projection() const = 0;
            virtual ~Camera() {};
        };

        struct Controller {
            Camera* camera;

            virtual void handle_sdl_events(const std::vector<SDL_Event>& sdl_events) = 0;
            virtual void delta_time_update(float delta_time) = 0;
            virtual ~Controller() {};
        };
        
        class Fog {
            glm::vec3 color_ {0.4f};
            float near_ {0.1f};
            float far_ {100.0f};

            public:
                Fog() {}
                ~Fog() = default;

                void set(const render::Material& material);
                void set_color(const render::Material& material, const glm::vec3& color);
                void set_near(const render::Material& material, const float near);
                void set_far(const render::Material& material, const float far);
        };

        class Directional_Light {
            glm::vec3 position_ {1.2f, 1.0f, 2.0f};
            glm::vec3 direction_ {-0.2f, -1.0f, -0.3f};
            glm::vec3 color_ {300};

            public:
                Directional_Light() {}
                ~Directional_Light() = default;

                void set(const render::Material& material);
                void set_color(const render::Material& material, const glm::vec3& color);
                void set_position(const render::Material& material, const glm::vec3& position);
                void set_direction(const render::Material& material, const glm::vec3& direction);
        };
    }

    class Scene {
        protected:
            glm::vec2 screen_dimensions_;
            glm::vec3 background_color_;

            std::map<std::string, std::unique_ptr<scene::Renderable>> renderables_;
            std::map<std::string, std::unique_ptr<scene::Controller>> controllers_;
            std::map<std::string, std::unique_ptr<scene::Camera>> cameras_;

            scene::Controller* current_controller_ {nullptr};
            scene::Camera* current_camera_ {nullptr};

            scene::Fog fog_ {};
            scene::Directional_Light light_ {};

            std::function<void()> callback_ {nullptr};

        public:
            Scene(const glm::vec2& screen_dimensions);

            void loop(float delta_time, const std::vector<SDL_Event>& sdl_events);

            void set_background_color(const glm::vec3& color) { background_color_ = color; }
            void set_callback(const std::function<void()> callback) { callback_ = callback; }
            void set_controller(const std::string& name);
            void set_camera(const std::string& name);

            void set_fog(const glm::vec3& color, const float near, const float far);
            void set_fog_color(const glm::vec3& color);
            void set_fog_near(const float near);
            void set_fog_far(const float far);
    
            void set_light(const glm::vec3& position, const glm::vec3& direction, const glm::vec3& color);
            void set_light_position(const glm::vec3& position);
            void set_light_direction(const glm::vec3& direction);
            void set_light_color(const glm::vec3& color);

            scene::Renderable* get_renderable(const std::string& name) { return renderables_.at(name).get(); }
            scene::Controller* get_controller(const std::string& name) { return controllers_.at(name).get(); }
            scene::Camera* get_camera(const std::string& name) { return cameras_.at(name).get(); }
            scene::Fog* get_fog() { return &fog_; }
            scene::Directional_Light* get_light() { return &light_; }

            template<typename T>
            void add_camera(const std::string& name, std::unique_ptr<T>& object) {
                static_assert(std::is_base_of<scene::Camera, T>::value);
                object->setup_dimensions(screen_dimensions_);
                cameras_.insert_or_assign(name, std::move(object));
            }

            template<typename T>
            void add_renderable(const std::string& name, std::unique_ptr<T>& object) {
                static_assert(std::is_base_of<scene::Renderable, T>::value);
                for(auto& [prim, mat]: object->primitives) {
                    light_.set(mat);
                    fog_.set(mat);
                }
                renderables_.insert_or_assign(name, std::move(object));
            }

            template<typename T>
            void add_controller(const std::string& name, std::unique_ptr<T>& object) {
                static_assert(std::is_base_of<scene::Controller, T>::value);
                controllers_.insert_or_assign(name, std::move(object));
            }
    };
}
