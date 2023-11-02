#include "glp/sdl.hh"
#include "glp/model.hh"
#include "glp/shader.hh"
#include "glp/anim.hh"

#include "glp/external/glm/glm.hpp"
#include "glp/external/glm/gtc/matrix_transform.hpp"
#include "glp/external/glm/gtc/type_ptr.hpp"

int main(int argc, char* argv[]) {
    if(argc!=3) glp_log("[model path] [assimp (1) or protobuf (0)]"), exit(1);
    Window sdl {"glp", 1280, 720};

    Shader shader{"../res/shaders/skinned.vert", "../res/shaders/skinned.frag"};
    Model model (argv[1], atoi(argv[2]), &shader);
    Animation::Animation animation (argv[1], model, atoi(argv[2]));
    Animation::Animator animator {&animation, &model};

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    auto cam_pos = glm::vec3(0.0f, 1.0f, 3.0f);
    auto cam_front = glm::vec3(0.0f, 0.0f, -1.0f);
    auto cam_up = glm::vec3(0.0f, 1.0f, 0.0f);
    float cam_yaw = -90.0f;
    bool moving = false;

    while(sdl.is_running()) {
        float spd = 100.0f * sdl.get_dt();
        while(SDL_PollEvent(&sdl.event) != 0) {
            if(sdl.event.type == SDL_QUIT) sdl.stop();
            if(sdl.event.type == SDL_KEYDOWN) {
                if(sdl.event.key.keysym.sym == SDLK_w) cam_pos += spd * cam_front;
                if(sdl.event.key.keysym.sym == SDLK_s) cam_pos -= spd * cam_front;
                if(sdl.event.key.keysym.sym == SDLK_a) cam_pos -= glm::normalize(glm::cross(cam_front, cam_up)) * spd;
                if(sdl.event.key.keysym.sym == SDLK_d) cam_pos += glm::normalize(glm::cross(cam_front, cam_up)) * spd;
                if(sdl.event.key.keysym.sym == SDLK_q) cam_yaw -= spd; cam_front = glm::normalize(glm::vec3(cos(glm::radians(cam_yaw)), 0.0f, sin(glm::radians(cam_yaw))));
                if(sdl.event.key.keysym.sym == SDLK_e) cam_yaw += spd; cam_front = glm::normalize(glm::vec3(cos(glm::radians(cam_yaw)), 0.0f, sin(glm::radians(cam_yaw))));
            }
        }
        sdl.loop_start();
        if(util::glerr()) glp_log("CLEAR");

        auto view = glm::lookAt(cam_pos, cam_pos+cam_front, cam_up);
        auto projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(90.0f), 
                    (float)sdl.get_width()/(float)sdl.get_height(), 0.1f, 100000.0f);
        //view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        auto m = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
        auto mvp = projection * view * m;

        shader.bind();
        shader.set("mvp", mvp);
        auto transforms = animator.get_bone_matrices();
        for(size_t i=0; i<transforms.size(); i++)
            shader.set("pose["+std::to_string(i)+"]", transforms[i]);
        animator.update(sdl.get_dt());
        model.render();
        util::glerr();

        sdl.loop_end();
    }

    return 0;
}
