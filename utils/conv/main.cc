#include <fstream>
#include <sstream>
#include <string>
#include "utils.hh"
#include "model.hh"
#include "shader.hh"
#include "sdl.hh"
#include "anim.hh"

extern const unsigned char phong_frag[];
extern const unsigned char static_vert[];

int main(int argc, char* argv[]) {

    if(argc != 4) {
        fprintf(stderr, "usage: %s [1 - model; 0 - anim] [model path] [output file]", argv[0]);
        return 1;
    }

    glp::Window sdl {"glp-util", 1, 1};
    glp::Shader sh {reinterpret_cast<const char*>(static_vert), reinterpret_cast<const char*>(phong_frag), false};

    if(atoi(argv[1])) {
        auto model = glp::Model(argv[2], &sh, glp::ShadingType::PHONG);
        std::fstream output(argv[3], std::ios::out | std::ios::trunc);
        std::stringstream data = model.serialize_data();
        auto compressed = glp::util::compress(data.str(), 90);
        output << compressed;
    } else {
        auto model = glp::Model(argv[2], &sh, glp::ShadingType::PHONG);
        auto anim = glp::Animation::Animation(argv[2], model);
        std::fstream output(argv[3], std::ios::out | std::ios::trunc);
        std::stringstream data = anim.serialize_data();
        auto compressed = glp::util::compress(data.str(), 90);
        output << compressed;
    }

    return 0;
}
