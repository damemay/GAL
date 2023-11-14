#include "glp/sdl.hh"
#include "glp/model.hh"
#include "glp/shader.hh"
#include "glp/anim.hh"

#include "glp/external/glm/glm.hpp"
#include "glp/external/glm/gtc/matrix_transform.hpp"
#include "glp/external/glm/gtc/type_ptr.hpp"

#include "glp/obj/camera.hh"
#include "glp/obj/player.hh"
#include "glp/fonts.hh"

#include "glp/external/imgui/imgui.h"
#include "glp/external/imgui/imgui_impl_sdl2.h"
#include "glp/external/imgui/imgui_impl_opengl3.h"

#include <sstream>

constexpr size_t WIDTH = 1280;
constexpr size_t HEIGHT = 720;

static Model model;
static Shader* shader;
static int is_phong = 1;

int main(int argc, char* argv[]) {
    Window sdl {"glp", WIDTH, HEIGHT};
    sdl.set_bg_color(glm::vec3(0.4, 0.4, 0.4));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(*sdl.get_window(), sdl.get_glcontext());
    ImGui_ImplOpenGL3_Init();

    Object::Camera camera {glm::vec2(1280.0f, 720.0f), 90.0f, 0.1, 2000.0f};
    Object::PlayerFPP player {&camera, &sdl.events};
    player.use_mouse(true);

    Shader phong {"../res/shaders/static.vert", "../res/shaders/phong.frag"};
    Shader pbr {"../res/shaders/static.vert", "../res/shaders/pbr.frag"};
    shader = &phong;
    model.set_shader(shader);
    model.set_shading_type(ShadingType::PHONG);

    model.load("../res/glTF-Sample-Models/2.0/Cube/glTF/Cube.gltf");

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    //SDL_SetRelativeMouseMode(SDL_TRUE);

    while(sdl.is_running()) {
        sdl.loop_start();
        for(auto& e: sdl.events) ImGui_ImplSDL2_ProcessEvent(&e);
        if(util::glerr()) glp_log("CLEAR");

        player.fpp_movement_keys();
        player.fpp_movement(*sdl.get_dt_ptr());

        if(is_phong) {
            shader = &phong;
            model.set_shader(shader);
            model.set_shading_type(ShadingType::PHONG);
        } else {
            shader = &pbr;
            model.set_shader(shader);
            model.set_shading_type(ShadingType::PBR);
        }

        shader->bind();
        auto c_vp = camera.view_projection();
        auto vp = c_vp.second * c_vp.first;
        shader->set("vp", vp);
        auto m = glm::mat4(1.0f);
        shader->set("model", m);
        model.render();
        shader->unbind();

        util::glerr();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(250, 680));
        ImGui::Begin("glp");

        if(ImGui::TreeNode("Light")) {
            if(ImGui::TreeNode("Direction")) {
                static float x = -0.2f;
                static float y = -1.0f;
                static float z = -0.3f;
                if(ImGui::InputFloat("X", &x, 0.1f) || ImGui::InputFloat("Y", &y, 0.1f)
                        || ImGui::InputFloat("Z", &z, 0.1f))
                    shader->set("light.direction", glm::vec3(x, y, z));
                ImGui::TreePop();
            }
            if(is_phong) {
                if(ImGui::TreeNode("Ambient")) {
                    static float x = 0.2f;
                    static float y = 0.2f;
                    static float z = 0.2f;
                    if(ImGui::InputFloat("R", &x, 0.1f) || ImGui::InputFloat("G", &y, 0.1f)
                            || ImGui::InputFloat("B", &z, 0.1f))
                        shader->set("light.ambient", glm::vec3(x, y, z));
                    ImGui::TreePop();
                }
                if(ImGui::TreeNode("Diffuse")) {
                    static float x = 0.5f;
                    static float y = 0.5f;
                    static float z = 0.5f;
                    if(ImGui::InputFloat("R", &x, 0.1f) || ImGui::InputFloat("G", &y, 0.1f)
                            || ImGui::InputFloat("B", &z, 0.1f))
                        shader->set("light.diffuse", glm::vec3(x, y, z));
                    ImGui::TreePop();
                }
                if(ImGui::TreeNode("Specular")) {
                    static float x = 0.7f;
                    static float y = 0.7f;
                    static float z = 0.7f;
                    if(ImGui::InputFloat("R", &x, 0.1f) || ImGui::InputFloat("G", &y, 0.1f)
                            || ImGui::InputFloat("B", &z, 0.1f))
                        shader->set("light.specular", glm::vec3(x, y, z));
                    ImGui::TreePop();
                }
            } else {
                if(ImGui::TreeNode("Color")) {
                    static float x = 300.0f;
                    static float y = 300.0f;
                    static float z = 300.0f;
                    if(ImGui::InputFloat("R", &x, 1.0f) || ImGui::InputFloat("G", &y, 1.0f)
                            || ImGui::InputFloat("B", &z, 1.0f))
                        shader->set("light.color", glm::vec3(x, y, z));
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Model")) {
            static char buf[2048];
            ImGui::InputText("path", buf, IM_ARRAYSIZE(buf));
            if(ImGui::Button("Load")) {
                model.load(buf);
            }
            ImGui::TreePop();
        }

        ImGui::RadioButton("PHONG", &is_phong, 1);
        ImGui::SameLine();
        ImGui::RadioButton("PBR", &is_phong, 0);

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        sdl.loop_end();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
