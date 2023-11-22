#include <fstream>
#include <sstream>
#include <string>
#include "utils.hh"
#include "model.hh"
#include "shader.hh"
#include "sdl.hh"
#include "anim.hh"
#include <obj/builtin-shaders.hh>

int main(int argc, char* argv[]) {

    if(argc != 4) {
        fprintf(stderr, "usage: %s [1 - model; 0 - anim] [model path] [output file]", argv[0]);
        return 1;
    }

    glp::Window sdl {"glp-util", 1, 1};
    auto [sh, sh_t] = glp::Object::make_static_phong();

    if(atoi(argv[1])) {
        auto model = glp::Model(argv[2], sh, sh_t);
        std::fstream output(argv[3], std::ios::out | std::ios::trunc);
        std::stringstream data = model.serialize_data();
        auto compressed = glp::util::compress(data.str(), 90);
        output << compressed;
    } else {
        auto model = glp::Model(argv[2], sh, sh_t);
        auto anim = glp::Animation::Animation(argv[2], model);
        std::fstream output(argv[3], std::ios::out | std::ios::trunc);
        std::stringstream data = anim.serialize_data();
        auto compressed = glp::util::compress(data.str(), 90);
        output << compressed;
    }

    return 0;
}
