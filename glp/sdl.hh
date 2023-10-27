#pragma once

#include <vector>
#include <optional>
#include <string>

#include <SDL2/SDL.h>

#ifndef __vita__
    #include "external/glad/glad.h"
    #ifdef __APPLE__
        #include <OpenGL/gl.h>
    #else
        #include <GL/gl.h>
    #endif
#else
    #include <vitaGL.h>
#endif

#include "utils.h"

class Window {
    private:
        SDL_Window* window {nullptr};
        std::vector<SDL_GameController*> gamepads;
        SDL_GLContext context {nullptr};

        size_t width, height;
        
        bool run {true};
        uint64_t start_tick {0}, last_tick {0};
        float dt {0.0f};

        void init(const uint32_t flags);
        std::optional<std::vector<SDL_GameController*>> gamepads_init();
        void gamepads_clean(const std::vector<SDL_GameController*>& gamepads);
        SDL_Window* window_init(const std::string title, const size_t width, const size_t height, const uint32_t flags);
        void gl_init();
        void create_glcontext(SDL_Window*& window, SDL_GLContext& context, const size_t width, const size_t height);

    public:
        SDL_Event event;

        Window(std::string title, const size_t width, const size_t height);
        ~Window();

        inline bool is_running() { return run; }
        inline void stop() { run = false; }
        inline float get_dt() { return dt; }

        inline void loop_start() {
            glClearColor(0.0f, 0.5f, 0.9f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            start_tick = SDL_GetTicks64();
            dt = (start_tick - last_tick) / 1000.0f;
        }

        inline void loop_end() {
            last_tick = start_tick;
            SDL_GL_SwapWindow(window);
        }
};
