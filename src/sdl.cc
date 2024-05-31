#include <SDL2/SDL_video.h>
#include <sdl.hh>
#include <format>
#include <stdexcept>
#include <gl.hh>

namespace glp {
    namespace sdl {
        void Window::init(const uint32_t flags) {
            if(SDL_Init(flags) > 0) {
                auto exception = std::format("could not init SDL: {}", SDL_GetError());
                throw std::runtime_error(exception);
            }
            atexit(SDL_Quit);
        }
        
        void Window::window_init(const uint32_t flags) {
            window_ = SDL_CreateWindow(title_.c_str(),
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            width_, height_, flags);
            if(!window_) {
                auto exception = std::format("could not create SDL Window: {}", SDL_GetError());
                throw std::runtime_error(exception);
            }
        }
        
        Window::Window(const std::string& title, const size_t width, const size_t height, const uint32_t sdl_flags, const uint32_t window_flags) : title_{title}, width_{width}, height_{height} {
            init(sdl_flags);
            window_init(window_flags);
            opengl::load(window_, context_, width, height);
        }
        
        Window::~Window() {
            SDL_GL_DeleteContext(context_);
            SDL_DestroyWindow(window_);
        }
    }
}
