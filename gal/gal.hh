#pragma once

#include <map>
#include <memory>
#include <functional>
#include <sdl.hh>
#include <scene.hh>

namespace gal {
    class GAL {
        sdl::Window window_;
        bool running_ {true};
        float delta_time_ {0.0f};
        uint64_t start_tick_ {0}, last_tick_ {0};
        std::vector<SDL_Event> sdl_events_;
        std::map<std::string, std::unique_ptr<Scene>> scenes_;
        Scene* current_scene_ {nullptr};

        public:
            GAL(const std::string& window_title, const size_t window_width, const size_t window_height);
            ~GAL() = default;

            void loop(const std::function<void()>& callback);
            void stop() { running_ = false; }

            const std::vector<SDL_Event>& get_sdl_events() { return sdl_events_; }
            sdl::Window& get_window() { return window_; }

            Scene* get_scene(const std::string& name);
            void set_scene(const std::string& name);
            Scene* add_scene(const std::string& name);
    };
}
