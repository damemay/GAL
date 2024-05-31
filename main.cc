#include <glp.hh>

constexpr int width = 1280;
constexpr int height = 720;

int main(int argc, char* argv[]) {
    glp::GLP sample {"glp-bullet", width, height};

    sample.loop(nullptr);

    return EXIT_SUCCESS;
}
