#include <fstream>
#include <sstream>
#include <string>
#include "../../glp/utils.hh"
#include "../../glp/model.hh"
#include "../../glp/shader.hh"
#include "../../glp/sdl.hh"
#include "../../glp/anim.hh"

int main(int argc, char* argv[]) {

    if(argc != 4) {
        fprintf(stderr, "usage: %s [1 - model; 0 - anim] [model path] [output file]", argv[0]);
        return 1;
    }

    glp::Window sdl {"glp-util", 1, 1};

    if(atoi(argv[1])) {
        auto model = glp::Model(argv[2]);
        std::fstream output(argv[3], std::ios::out | std::ios::trunc);
        std::stringstream data = model.serialize_data();
        auto compressed = glp::util::compress(data.str(), 90);
        output << compressed;
    } else {
        auto model = glp::Model(argv[2]);
        auto anim = glp::Animation::Animation(argv[2], model, true);
        std::fstream output(argv[3], std::ios::out | std::ios::trunc);
        std::stringstream data = anim.serialize_data();
        auto compressed = glp::util::compress(data.str(), 90);
        output << compressed;
    }

    return 0;
}
