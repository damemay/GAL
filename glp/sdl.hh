#pragma once

#include <vector>
#include <optional>
#include <string>

#include <SDL2/SDL.h>

#include <glad/glad.h>
#ifdef __APPLE__
    #include <OpenGL/gl.h>
#else
    #include <GL/gl.h>
#endif

#include <utils.hh>
#include <glm/glm.hpp>

namespace glp {

class Window {
    private:
        SDL_Window* window {nullptr};
        std::vector<SDL_GameController*> gamepads;
        SDL_GLContext context {nullptr};

        size_t width, height;
        
        bool run {true};
        float dt {0.0f};
        uint64_t start_tick {0}, last_tick {0};

        glm::vec3 bg {0.0f, 0.5, 0.9f};

        void init(const uint32_t flags);
        void gamepads_init();
        void window_init(const std::string title, const size_t width, const size_t height, const uint32_t flags);
        void gl_init();
        void create_glcontext(SDL_Window*& window, SDL_GLContext& context, const size_t width, const size_t height);

    public:
        std::vector<SDL_Event> events;
        Window(std::string title, const size_t width, const size_t height);
        ~Window();

        inline bool is_running() { return run; }
        inline void stop() { run = false; }

        inline float* get_dt_ptr() { return &dt; }

        inline size_t get_width() { return width; }
        inline size_t get_height() { return height; }
        inline SDL_Window** get_window() { return &window; }
        inline SDL_GLContext* get_glcontext() { return &context; }

        inline void set_bg_color(const glm::vec3& color) { bg = color; }

        inline void loop_start() {
            SDL_Event event;
            while(SDL_PollEvent(&event) != 0) {
                if(event.type == SDL_QUIT) stop();
                else events.push_back(event);
            }
            glClearColor(bg.x, bg.y, bg.z, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            start_tick = SDL_GetTicks64();
            dt = (start_tick - last_tick) / 1000.0f;
        }

        inline void loop_end() {
            last_tick = start_tick;
            events.clear();
            SDL_GL_SwapWindow(window);
        }
};

}
