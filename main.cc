#include "glp/sdl.hh"
#include "glp/model.hh"

int main(int argc, char* argv[]) {
    Window sdl {"glp", 960, 544};

    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    std::string path {"res/models/cube/untitled.gltf"};
    if(!loader.LoadASCIIFromFile(&model, &err, &warn, path))
        glp_logv("could not load %s", path.c_str());

    glEnable(GL_DEPTH_TEST);

    while(sdl.is_running()) {
        while(SDL_PollEvent(&sdl.event) != 0) {
            if(sdl.event.type == SDL_QUIT) sdl.stop();
        }
        sdl.loop_start();
        sdl.loop_end();
    }

    return 0;
}
