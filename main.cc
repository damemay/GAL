#include <glp.hh>
#include <camera.hh>
#include <controller.hh>
#include <model.hh>
#include <gl.hh>
#include <memory>

constexpr int width = 1280;
constexpr int height = 720;

int main(int argc, char* argv[]) {
    glp::GLP sample {"glp-bullet", width, height};

    auto scene = sample.add_scene("Sample");
    scene->set_background_color(glm::vec3{0.2f, 0.5f, 1.0f});

    auto camera {std::make_unique<glp::Basic_Camera>()};
    scene->add_camera("Main Camera", camera);

    auto fpp {std::make_unique<glp::Basic_FPP_Controller>(25.0f, 0.1f)};
    scene->add_controller("Player", fpp);

    auto model {std::make_unique<glp::Model>("/home/mar/glTF-Sample-Models/2.0/Triangle/glTF/Triangle.gltf")};
    scene->add_renderable("Triangle", model);

    scene->set_camera("Main Camera");
    scene->set_controller("Player");
    sample.set_scene("Sample");

    const char* vertex = "#version 410 core\n"
        "layout (location=0) in vec3 pos;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view_projection;\n"
        "void main() {\n"
        "gl_Position = view_projection * model * vec4(pos, 1.0f);\n"
        "}";
    const char* fragment = "#version 410 core\n"
        "out vec4 out_color;\n"
        "void main() {\n"
        "out_color = vec4(1.0, 0.5f, 0.2f, 1.0f);\n"
        "}";
    auto gl_shader = glp::opengl::load_shader(vertex, fragment, false);
    glm::mat4 pos {1.0f};

    auto main_camera = scene->get_camera("Main Camera");
    glUseProgram(gl_shader);
    sample.loop([gl_shader, main_camera, pos] {
        glUniformMatrix4fv(glGetUniformLocation(gl_shader, "view_projection"), 1, GL_FALSE, &main_camera->view_projection()[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(gl_shader, "model"), 1, GL_FALSE, &pos[0][0]);
    });
    glDeleteProgram(gl_shader);

    return EXIT_SUCCESS;
}
