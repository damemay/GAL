#pragma once

#include <SDL2/SDL_video.h>
#include <vector>
#include <string>

#include <SDL2/SDL.h>

#include <glad/glad.h>
#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#include <glm/glm.hpp>

namespace gal {
    namespace sdl {
        class Window {
            SDL_Window* window_ {nullptr};
            SDL_GLContext context_;
        
            std::string title_;
            size_t width_;
            size_t height_;
        
            void init(const uint32_t flags);
            void window_init(const uint32_t flags);
        
            public:
                std::vector<SDL_Event> events;
                Window(const std::string& title, const size_t width, const size_t height, const uint32_t sdl_flags, const uint32_t window_flags);
                ~Window();
        
                inline size_t get_width() { return width_; }
                inline size_t get_height() { return height_; }
                inline SDL_Window* get_sdl_window() { return window_; }
                inline SDL_GLContext& get_glcontext() { return context_; }
        };
    }
}
