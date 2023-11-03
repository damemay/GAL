#include <fstream>
#include <sstream>
#include <string>
#include "../glp/utils.hh"
#include "../glp/model.hh"
#include "../glp/shader.hh"
#include "../glp/sdl.hh"

int main(int argc, char* argv[]) {

    if(argc != 3) {
        fprintf(stderr, "usage: %s [model path] [output file]", argv[0]);
        return 1;
    }

    Window sdl {"glp-util", 1, 1};

    Model model = Model(argv[1], true, nullptr);

    std::fstream output(argv[2], std::ios::out | std::ios::trunc);
    output << model.serialize_data().rdbuf();

    return 0;
}
