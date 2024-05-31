#pragma once

#include <map>
#include <type_traits>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

namespace glp {
    namespace scene {
        struct Renderable {
            Renderable();
            virtual void render() = 0;
            virtual ~Renderable() {};
        };

        struct Controller {
            virtual ~Controller() {};
        };
        
        struct Camera {
            virtual ~Camera() {};
        };
    }

    class Scene {
        glm::vec2 screen_dimensions_;
        glm::vec3 background_color_;

        std::map<std::string, std::unique_ptr<scene::Renderable>> renderables_;
        std::map<std::string, std::unique_ptr<scene::Controller>> controllers_;
        std::map<std::string, std::unique_ptr<scene::Camera>> cameras_;

        scene::Controller* current_controller_;
        scene::Camera* current_camera_;

        public:
            Scene(const glm::vec2& screen_dimensions);

            void loop(float delta_time, const std::vector<SDL_Event>& sdl_events);

            inline void set_background_color(const glm::vec3& color) { background_color_ = color; }

            inline scene::Renderable* get_renderable(const std::string& name) { return renderables_.at(name).get(); }
            inline scene::Controller* get_controller(const std::string& name) { return controllers_.at(name).get(); }
            inline scene::Camera* get_camera(const std::string& name) { return cameras_.at(name).get(); }
            inline void set_controller(const std::string& name) { current_controller_ = controllers_.at(name).get(); }
            inline void set_camera(const std::string& name) { current_camera_ = cameras_.at(name).get(); }

            template<typename R, typename std::enable_if<std::is_base_of<scene::Renderable, R>::value>::type>
            inline void add(const std::string& name, R& object) {
                renderables_.insert_or_assign(name, std::make_unique<R>(object));
            }

            template<typename Cr, typename std::enable_if<std::is_base_of<scene::Controller, Cr>::value>::type>
            inline void add(const std::string& name, Cr& object) {
                controllers_.insert_or_assign(name, std::make_unique<Cr>(object));
            }

            template<typename Ca, typename std::enable_if<std::is_base_of<scene::Camera, Ca>::value>::type>
            inline void add(const std::string& name, Ca& object) {
                controllers_.insert_or_assign(name, std::make_unique<Ca>(object));
            }
    };
}
