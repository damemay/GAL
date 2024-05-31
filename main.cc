#include <glp.hh>
#include <camera.hh>
#include <memory>

constexpr int width = 1280;
constexpr int height = 720;

int main(int argc, char* argv[]) {
    glp::GLP sample {"glp-bullet", width, height};

    auto scene = sample.add_scene("Sample");
    scene->set_background_color(glm::vec3{0.2f, 0.5f, 1.0f});

    auto camera {std::make_unique<glp::Basic_Camera>()};
    scene->add_camera("Main Camera", camera);

    sample.set_scene("Sample");
    sample.loop(nullptr);

    return EXIT_SUCCESS;
}
