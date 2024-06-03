#include <gal.hh>
#include <camera.hh>
#include <controller.hh>
#include <model.hh>
#include <gl.hh>
#include <memory>

int main(int argc, char* argv[]) {
    gal::GAL sample {"GAL sample", 1280, 720};

    auto scene = sample.add_scene("Sample");
    scene->set_background_color(glm::vec3{0.2f, 0.5f, 1.0f});

    {
        auto camera {std::make_unique<gal::Basic_Camera>()};
        scene->add_camera("Main Camera", camera);

        auto fpp {std::make_unique<gal::Basic_FPP_Controller>(100.0f, 0.1f)};
        scene->add_controller("Player", fpp);

        auto model {std::make_unique<gal::GLTF_Render_Model>("/home/mar/glTF-Sample-Models/2.0/Sponza/glTF/Sponza.gltf")};
        scene->add_renderable("Model", model);
    }

    {
        auto camera {scene->get_camera("Main Camera")};
        camera->far = 1000.0f;

        scene->set_fog_far(1000.0f);
        scene->set_fog_color(glm::vec3{0.2f, 0.5f, 1.0f});
    }

    scene->set_camera("Main Camera");
    scene->set_controller("Player");
    sample.set_scene("Sample");

    sample.loop(nullptr);

    return EXIT_SUCCESS;
}
