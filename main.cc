#include "glp/sdl.hh"
#include "glp/model.hh"
#include "glp/shader.hh"

#include "glp/external/glm/glm.hpp"
#include "glp/external/glm/gtc/matrix_transform.hpp"
#include "glp/external/glm/gtc/type_ptr.hpp"

int main(int argc, char* argv[]) {
    if(argc!=3) glp_log("[model path] [assimp (1) or protobuf (0)]"), exit(1);
    Window sdl {"glp", 1280, 720};

    Shader shader{"../res/shaders/cube.vert", "../res/shaders/cube.frag"};
    Model gltf (argv[1], atoi(argv[2]));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    auto cam_pos = glm::vec3(0.0f, 0.0f, 3.0f);
    auto cam_front = glm::vec3(0.0f, 0.0f, -1.0f);
    auto cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    float cam_yaw = -90.0f;
    bool moving = false;

    while(sdl.is_running()) {
        float spd = 50.0f * sdl.get_dt();
        while(SDL_PollEvent(&sdl.event) != 0) {
            if(sdl.event.type == SDL_QUIT) sdl.stop();
            if(sdl.event.type == SDL_KEYDOWN) {
                switch(sdl.event.key.keysym.sym) {
                    case SDLK_w: cam_pos += spd * cam_front; break;
                    case SDLK_s: cam_pos -= spd * cam_front; break;
                    case SDLK_a: cam_pos -= glm::normalize(glm::cross(cam_front, cam_up)) * spd; break;
                    case SDLK_d: cam_pos += glm::normalize(glm::cross(cam_front, cam_up)) * spd; break;
                    case SDLK_q: cam_yaw -= spd; cam_front = glm::normalize(glm::vec3(cos(glm::radians(cam_yaw)), 0.0f, sin(glm::radians(cam_yaw)))); break;
                    case SDLK_e: cam_yaw += spd; cam_front = glm::normalize(glm::vec3(cos(glm::radians(cam_yaw)), 0.0f, sin(glm::radians(cam_yaw)))); break;
                }
            }
        }
        sdl.loop_start();
        if(util::glerr()) glp_log("CLEAR");

        glm::mat4 view = glm::lookAt(cam_pos, cam_pos+cam_front, cam_up);
        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(60.0f), 
                    (float)sdl.get_width()/(float)sdl.get_height(), 0.1f, 1000.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        glm::mat4 mvp = projection * view * model;

        shader.bind();
        shader.set("mvp", mvp);
        gltf.render(shader);

        sdl.loop_end();
    }

    return 0;
}
