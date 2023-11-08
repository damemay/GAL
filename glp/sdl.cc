#include "sdl.hh"
#include <SDL2/SDL_ttf.h>

void Window::init(const uint32_t flags) {
    if(SDL_Init(flags) > 0) {
        glp_diev("could not init SDL: %s", SDL_GetError());
    }
    atexit(SDL_Quit);
}

std::optional<std::vector<SDL_GameController*>> Window::gamepads_init() {
    int joysticks;
    if((joysticks=SDL_NumJoysticks()) > 0) {
        std::vector<SDL_GameController*> gamepads(joysticks);
        for(size_t i=0; i<joysticks; ++i)
            gamepads.at(i) = SDL_GameControllerOpen(i);
        return gamepads;
    } else return {};
}

void Window::gamepads_clean(const std::vector<SDL_GameController*>& gamepads) {
    for(const auto& pad : gamepads)
        SDL_GameControllerClose(pad);
}

SDL_Window* Window::window_init(const std::string title, const size_t width, const size_t height, const uint32_t flags) {
    SDL_Window* window = nullptr;
    if(!(window = SDL_CreateWindow(title.c_str(),
                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                    width, height, flags)))
        glp_diev("could not create SDL Window: %s", SDL_GetError());
    return window;
}

void Window::gl_init() {
#ifndef __vita__
    SDL_GL_LoadLibrary(NULL);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
    SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
    SDL_GL_CONTEXT_PROFILE_CORE);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
}

void Window::create_glcontext(SDL_Window*& window, SDL_GLContext& context, const size_t width, const size_t height) {
    context = SDL_GL_CreateContext(window);
    if(!context)
        glp_diev("could not create SDL_GLContext: %s", SDL_GetError());
    SDL_GL_MakeCurrent(window, context);

#ifndef __vita__
    if(!gladLoadGLLoader(SDL_GL_GetProcAddress))
        glp_die("could not load glad!");
#endif

    glViewport(0, 0, width, height);
    glClearColor(0.0f, 0.0f, 0.9f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

Window::Window(std::string title, const size_t width_, const size_t height_) : width{width_}, height{height_} {
    init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
    window = window_init(title, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    gl_init();
    create_glcontext(window, context, width, height);
    if(auto pads = gamepads_init())
        gamepads = std::move(*pads);
    if(TTF_Init() < 0) glp_diev("could not init SDL_ttf: %s", TTF_GetError());
    atexit(TTF_Quit);
}

Window::~Window() {
    if(!gamepads.empty()) gamepads_clean(gamepads);
    if(context) SDL_GL_DeleteContext(context);
    if(window) SDL_DestroyWindow(window);
}
