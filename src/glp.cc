#include <glp.hh>

namespace glp {
    GLP::GLP(const std::string& window_title, const size_t window_width, const size_t window_height) : window_{window_title, window_width, window_height, SDL_INIT_VIDEO, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL} {

    }

    void GLP::loop(const std::function<void()>& callback) {
        while(running_) {
            SDL_Event event;
            while(SDL_PollEvent(&event) != 0) {
                if(event.type == SDL_QUIT) stop();
                else sdl_events_.push_back(event);
            }
            start_tick_ = SDL_GetTicks64();
            delta_time_ = (start_tick_ - last_tick_) / 1000.0f;

            if(callback) callback();
            if(current_scene_) current_scene_->loop(delta_time_, sdl_events_);

            last_tick_ = start_tick_;
            sdl_events_.clear();
            SDL_GL_SwapWindow(window_.get_sdl_window());
        }
    }
}
