#include "sdl.hh"
#include "model.hh"
#include "shader.hh"
#include "anim.hh"

#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "external/glm/gtc/type_ptr.hpp"

#include "obj/camera.hh"
#include "obj/player.hh"
#include "fonts.hh"

#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_sdl2.h"
#include "external/imgui/imgui_impl_opengl3.h"
#include "utils.hh"

#include <sstream>
#include <fstream>

constexpr size_t WIDTH = 1280;
constexpr size_t HEIGHT = 720;

static Model* model;
static Shader* shader;
static int is_phong = 1;
static int is_directional = 1;

static float fr = 0.4f;
static float fg = 0.4f;
static float fb = 0.4f;
static float fn = 0.1f;
static float ff = 100.0f;

static float dirx = -0.2f;
static float diry = -1.0f;
static float dirz = -0.3f;

static float linear = 0.09f;
static float quadratic = 0.032f;

static float posx = 1.2f;
static float posy = 1.0f;
static float posz = 2.0f;

static float ax = 0.2f;
static float ay = 0.2f;
static float az = 0.2f;

static float difx = 0.5f;
static float dify = 0.5f;
static float difz = 0.5f;

static float sx = 0.7f;
static float sy = 0.7f;
static float sz = 0.7f;

static float cx = 300.0f;
static float cy = 300.0f;
static float cz = 300.0f;

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

    Object::Camera camera {glm::vec2(WIDTH, HEIGHT), 90.0f, 0.1, 2000.0f};
    Object::PlayerFPP player {&camera, &sdl.events};
    player.use_mouse(true);

    Shader phong {"../res/shaders/static.vert", "../res/shaders/phong.frag"};
    Shader pbr {"../res/shaders/static.vert", "../res/shaders/pbr.frag"};
    shader = &phong;

    model = new Model("../res/cube/Cube.gltf");
    model->set_shader(shader);
    model->set_shading_type(ShadingType::PHONG);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);


    while(sdl.is_running()) {
        sdl.loop_start();
        for(auto& e: sdl.events) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if(e.type == SDL_KEYUP) {
                if(e.key.keysym.sym == SDLK_q) SDL_SetRelativeMouseMode(SDL_TRUE);
                if(e.key.keysym.sym == SDLK_e) SDL_SetRelativeMouseMode(SDL_FALSE);
            }
        }
        if(util::glerr()) glp_log("GL Error left from last frame");

        player.fpp_movement_keys();
        player.fpp_movement(*sdl.get_dt_ptr());

        shader->bind();
        auto vp = camera.view_projection();
        shader->set("vp", vp);
        auto m = glm::mat4(1.0f);
        shader->set("model", m);
        model->render();

        if(util::glerr()) glp_log("Rendering did not pass without errors");

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(250, 680));
        ImGui::Begin("glp");

        if(ImGui::TreeNode("Fog")) {
            if(ImGui::TreeNode("Color")) {
                if(ImGui::InputFloat("R", &fr, 0.1f) || ImGui::InputFloat("G", &fg, 0.1f)
                        || ImGui::InputFloat("B", &fb, 0.1f))
                    shader->set("fog.color", glm::vec3(fr, fg, fb));
                ImGui::TreePop();
            }
            if(ImGui::InputFloat("Near", &fn, 0.1f) || ImGui::InputFloat("Far", &ff, 0.1f)) {
                shader->set("fog.near", fn);
                shader->set("fog.far", ff);
            }
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Light")) {
            ImGui::RadioButton("Directional", &is_directional, 1);
            ImGui::SameLine();
            ImGui::RadioButton("Point", &is_directional, 0);
            if(is_directional) {
                if(ImGui::TreeNode("Direction")) {
                    if(ImGui::InputFloat("X", &dirx, 0.1f) || ImGui::InputFloat("Y", &diry, 0.1f)
                            || ImGui::InputFloat("Z", &dirz, 0.1f))
                        shader->set("light.direction", glm::vec3(dirx, diry, dirz));
                    ImGui::TreePop();
                }
            } else {
                if(ImGui::TreeNode("Position")) {
                    if(ImGui::InputFloat("X", &posx, 0.1f) || ImGui::InputFloat("Y", &posy, 0.1f)
                            || ImGui::InputFloat("Z", &posz, 0.1f))
                        shader->set("light.position", glm::vec3(posx, posy, posz));
                    ImGui::TreePop();
                }
                if(is_phong) {
                    if(ImGui::TreeNode("Range")) {
                        if(ImGui::InputFloat("Linear", &linear, 0.01f) || ImGui::InputFloat("Quadratic", &quadratic, 0.01f)) {
                            shader->set("light.linear", linear);
                            shader->set("light.quadratic", quadratic);
                        }
                        ImGui::TreePop();
                    }
                }
            }
            if(is_phong) {
                if(ImGui::TreeNode("Ambient")) {
                    if(ImGui::InputFloat("R", &ax, 0.1f) || ImGui::InputFloat("G", &ay, 0.1f)
                            || ImGui::InputFloat("B", &az, 0.1f))
                        shader->set("light.ambient", glm::vec3(ax, ay, az));
                    ImGui::TreePop();
                }
                if(ImGui::TreeNode("Diffuse")) {
                    if(ImGui::InputFloat("R", &difx, 0.1f) || ImGui::InputFloat("G", &dify, 0.1f)
                            || ImGui::InputFloat("B", &difz, 0.1f))
                        shader->set("light.diffuse", glm::vec3(difx, dify, difz));
                    ImGui::TreePop();
                }
                if(ImGui::TreeNode("Specular")) {
                    if(ImGui::InputFloat("R", &sx, 0.1f) || ImGui::InputFloat("G", &sy, 0.1f)
                            || ImGui::InputFloat("B", &sz, 0.1f))
                        shader->set("light.specular", glm::vec3(sx, sy, sz));
                    ImGui::TreePop();
                }
            } else {
                if(ImGui::TreeNode("Color")) {
                    if(ImGui::InputFloat("R", &cx, 1.0f) || ImGui::InputFloat("G", &cy, 1.0f)
                            || ImGui::InputFloat("B", &cz, 1.0f))
                        shader->set("light.color", glm::vec3(cx, cy, cz));
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Model")) {
            static char buf[2048];
            ImGui::InputText("path", buf, IM_ARRAYSIZE(buf));
            if(ImGui::Button("Load")) {
                delete model;
                model = new Model(buf);
            }
            if(ImGui::TreeNode("Export")) {
                static char buf1[2048];
                ImGui::InputText("path", buf1, IM_ARRAYSIZE(buf1));
                if(ImGui::Button("Save")) {
                    std::fstream output(buf1, std::ios::out | std::ios::trunc);
                    std::stringstream data = model->serialize_data();
                    auto compressed = util::compress(data.str(), 90);
                    output << compressed;
                }
                ImGui::TreePop();
            }
            ImGui::TreePop();
        }

        ImGui::RadioButton("PHONG", &is_phong, 1);
        ImGui::SameLine();
        ImGui::RadioButton("PBR", &is_phong, 0);

        if(is_phong) {
            shader = &phong;
            model->set_shader(shader);
            model->set_shading_type(ShadingType::PHONG);
            shader->bind();
            shader->set("fog.color", glm::vec3(fr, fg, fb));
            shader->set("fog.near", fn);
            shader->set("fog.far", ff);
            if(is_directional) {
                shader->set("light.direction", glm::vec3(dirx, diry, dirz));
                shader->set("light.directional", 1);
            } else {
                shader->set("light.position", glm::vec3(posx, posy, posz));
                shader->set("light.directional", 0);
                shader->set("light.linear", linear);
                shader->set("light.quadratic", quadratic);
            }
            shader->set("light.ambient", glm::vec3(ax, ay, az));
            shader->set("light.diffuse", glm::vec3(difx, dify, difz));
            shader->set("light.specular", glm::vec3(sx, sy, sz));
        } else {
            shader = &pbr;
            model->set_shader(shader);
            model->set_shading_type(ShadingType::PBR);
            shader->bind();
            shader->set("fog.color", glm::vec3(fr, fg, fb));
            shader->set("fog.near", fn);
            shader->set("fog.far", ff);
            if(is_directional) {
                shader->set("light.direction", glm::vec3(dirx, diry, dirz));
                shader->set("light.directional", 1);
            } else {
                shader->set("light.position", glm::vec3(posx, posy, posz));
                shader->set("light.directional", 0);
            }
            shader->set("light.color", glm::vec3(cx, cy, cz));
        }

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
