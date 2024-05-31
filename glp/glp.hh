#pragma once

#include <map>
#include <memory>
#include <sdl.hh>
#include <scene.hh>
#include <functional>

namespace glp {
    class GLP {
        sdl::Window window_;
        bool running_ {true};
        float delta_time_ {0.0f};
        uint64_t start_tick_ {0}, last_tick_ {0};
        std::vector<SDL_Event> sdl_events_;
        std::map<std::string, std::unique_ptr<Scene>> scenes_;
        Scene* current_scene_ {nullptr};

        public:
            GLP(const std::string& window_title, const size_t window_width, const size_t window_height);
            ~GLP() = default;

            void loop(const std::function<void()>& callback);
            inline void stop() { running_ = false; }

            inline const std::vector<SDL_Event>& get_sdl_events() { return sdl_events_; }
            inline sdl::Window& get_window() { return window_; }

            inline Scene* get_scene(const std::string& name) { return scenes_.at(name).get(); }
            inline void set_scene(const std::string& name) { current_scene_ = scenes_.at(name).get(); }
            inline Scene* add_scene(const std::string& name) {
                scenes_.insert_or_assign(name, std::make_unique<Scene>(glm::vec2{window_.get_width(), window_.get_height()}));
                return get_scene(name);
            }
    };
}
