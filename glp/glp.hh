#pragma once

#include <sdl.hh>
#include <functional>

namespace glp {
    class GLP {
        sdl::Window window_;
        bool running_ {true};
        float delta_time_ {0.0f};
        uint64_t start_tick_ {0}, last_tick_ {0};
        std::vector<SDL_Event> sdl_events_;

        public:

            GLP(const std::string& window_title, const size_t window_width, const size_t window_height);
            ~GLP() = default;

            void loop(const std::function<void()>& callback);
            inline void stop() { running_ = false; }

            inline const std::vector<SDL_Event>& get_sdl_events() { return sdl_events_; }
            inline sdl::Window& get_window() { return window_; }
    };
}
