#include "obj/collidable.hh"
#include "obj/light.hh"
#include "sdl.hh"
#include "model.hh"
#include "shader.hh"
#include "anim.hh"
#include "obj/scene.hh"

#include "external/glm/glm.hpp"
#include "external/glm/gtc/matrix_transform.hpp"
#include "external/glm/gtc/type_ptr.hpp"

#include "obj/camera.hh"
#include "obj/player.hh"
#include "obj/builtin-shaders.hh"
#include "fonts.hh"

#include "external/imgui/imgui.h"
#include "external/imgui/imgui_impl_sdl2.h"
#include "external/imgui/imgui_impl_opengl3.h"
#include "utils.hh"

#include <sstream>
#include <fstream>
#include <string>

constexpr size_t WIDTH = 1280;
constexpr size_t HEIGHT = 720;

static glp::Shader* shader;
static glp::ShadingType shading_t;
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

struct transform {
    float x     {0.0f};
    float y     {0.0f};
    float z     {0.0f};
    float yaw   {0.0f};
    float pitch {0.0f};
    float roll  {0.0f};
};

static std::vector<transform*> transforms;

bool mouse = false;

int main(int argc, char* argv[]) {
    glp::Window sdl {"glp", WIDTH, HEIGHT};
    sdl.set_bg_color(glm::vec3(0.4, 0.4, 0.4));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(*sdl.get_window(), sdl.get_glcontext());
    ImGui_ImplOpenGL3_Init();

    glp::Object::Camera camera {glm::vec2(WIDTH, HEIGHT), 90.0f, 0.1, 2000.0f};
    glp::Object::PlayerFPP player {&camera, &sdl.events};
    player.use_mouse(true);

    [[maybe_unused]] auto [phong, _] = glp::Object::make_static_phong();
    [[maybe_unused]] auto [pbr, __] = glp::Object::make_static_pbr();
    shader = phong;
    shading_t = glp::ShadingType::PHONG;

    auto scene = glp::Object::Scene{WIDTH, HEIGHT, nullptr, shader, &camera, false};
    scene.set_debug(true);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);


    while(sdl.is_running()) {
        sdl.loop_start();
        for(auto& e: sdl.events) {
            ImGui_ImplSDL2_ProcessEvent(&e);
            if(e.type == SDL_KEYUP) {
                if(e.key.keysym.sym == SDLK_q) mouse = true, SDL_SetRelativeMouseMode(SDL_TRUE);
                if(e.key.keysym.sym == SDLK_e) mouse = false, SDL_SetRelativeMouseMode(SDL_FALSE);
            }
        }
        if(glp::util::glerr()) glp_log("GL Error left from last frame");

        if(mouse) {
            player.fpp_movement_keys();
            player.fpp_movement(*sdl.get_dt_ptr());
        }

        scene.update(*sdl.get_dt_ptr());

        if(glp::util::glerr()) glp_log("Rendering did not pass without errors");

        if(!mouse){
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(250, 680));
        ImGui::Begin("glp");

        if(ImGui::TreeNode("Fog")) {
            if(ImGui::TreeNode("Color")) {
                if(ImGui::InputFloat("R", &fr, 0.1f) || ImGui::InputFloat("G", &fg, 0.1f)
                        || ImGui::InputFloat("B", &fb, 0.1f))
                    scene.get_fog().set_color(glm::vec3(fr, fg, fb));
                ImGui::TreePop();
            }
            if(ImGui::InputFloat("Near", &fn, 0.1f) || ImGui::InputFloat("Far", &ff, 0.1f)) {
                scene.get_fog().set_far(ff);
                scene.get_fog().set_near(fn);
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
                        scene.get_light().set_direction(glm::vec3(dirx, diry, dirz));
                    ImGui::TreePop();
                }
            } else {
                if(ImGui::TreeNode("Position")) {
                    if(ImGui::InputFloat("X", &posx, 0.1f) || ImGui::InputFloat("Y", &posy, 0.1f)
                            || ImGui::InputFloat("Z", &posz, 0.1f))
                        scene.get_light().set_position(glm::vec3(posx, posy, posz));
                    ImGui::TreePop();
                }
                if(is_phong) {
                    if(ImGui::TreeNode("Range")) {
                        if(ImGui::InputFloat("Linear", &linear, 0.01f) || ImGui::InputFloat("Quadratic", &quadratic, 0.01f)) {
                            scene.get_light().set_linear(linear);
                            scene.get_light().set_quadratic(quadratic);
                        }
                        ImGui::TreePop();
                    }
                }
            }
            if(is_phong) {
                if(ImGui::TreeNode("Ambient")) {
                    if(ImGui::InputFloat("R", &ax, 0.1f) || ImGui::InputFloat("G", &ay, 0.1f)
                            || ImGui::InputFloat("B", &az, 0.1f))
                        scene.get_light().set_ambient(glm::vec3(ax, ay, az));
                    ImGui::TreePop();
                }
                if(ImGui::TreeNode("Diffuse")) {
                    if(ImGui::InputFloat("R", &difx, 0.1f) || ImGui::InputFloat("G", &dify, 0.1f)
                            || ImGui::InputFloat("B", &difz, 0.1f))
                        scene.get_light().set_diffuse(glm::vec3(difx, dify, difz));
                    ImGui::TreePop();
                }
                if(ImGui::TreeNode("Specular")) {
                    if(ImGui::InputFloat("R", &sx, 0.1f) || ImGui::InputFloat("G", &sy, 0.1f)
                            || ImGui::InputFloat("B", &sz, 0.1f))
                        scene.get_light().set_specular(glm::vec3(sx, sy, sz));
                    ImGui::TreePop();
                }
            } else {
                if(ImGui::TreeNode("Color")) {
                    if(ImGui::InputFloat("R", &cx, 1.0f) || ImGui::InputFloat("G", &cy, 1.0f)
                            || ImGui::InputFloat("B", &cz, 1.0f))
                        scene.get_light().set_color(glm::vec3(cx, cy, cz));
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Scene")) {
            for(size_t i=0; i<scene.get_objects().size(); i++) {
                if(ImGui::TreeNode(scene.get_objects().at(i)->get_model()->get_name().data())) {
                    auto obj = scene.get_objects().at(i);
                    auto t = transforms.at(i);
                    if(ImGui::InputFloat("X", &t->x, 0.1f) || ImGui::InputFloat("Y", &t->y, 0.1f) || ImGui::InputFloat("Z", &t->z, 0.1f))
                        obj->reset(btVector3(t->x,t->y,t->z), btQuaternion(t->yaw, t->pitch, t->roll));
                    if(ImGui::InputFloat("YAW", &t->yaw, 0.1f) || ImGui::InputFloat("PITCH", &t->pitch, 0.1f) || ImGui::InputFloat("ROLL", &t->roll, 0.1f))
                        obj->reset(btVector3(t->x,t->y,t->z), btQuaternion(t->yaw, t->pitch, t->roll));
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Import")) {
            static char buf[2048];
            ImGui::InputText("path", buf, IM_ARRAYSIZE(buf));
            if(ImGui::Button("Load")) {
                scene.new_object(new glp::Object::CollRenderableModel{buf, shader, shading_t, 0.0f, btVector3(0,0,0)});
                std::string sbuf = buf;
                scene.get_objects().back()->get_model()->set_name(sbuf.substr(sbuf.find_last_of('/'), sbuf.length())+std::to_string(scene.get_objects().size()));
                transforms.push_back(new transform);
            }
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Export")) {
            static char buf0[2048];
            ImGui::InputText("path", buf0, IM_ARRAYSIZE(buf0));
            static char buf1[2048];
            ImGui::InputText("name", buf1, IM_ARRAYSIZE(buf1));
            if(ImGui::Button("Save")) {
                std::vector<std::string> dirs;
                std::string name;
                for(size_t i=0;i<scene.get_objects().size();i++) {
                    bool skip = false;
                    auto model = scene.get_objects().at(i)->get_model();
                    for(auto& dir: dirs) if(dir==model->get_directory()) skip = true;
                    if(skip) { 
                        model->set_directory(name);
                        continue;
                    }
                    dirs.push_back(model->get_directory());
                    name = std::string(buf1) +std::to_string(i)+".model";
                    auto path = std::string(buf0) + '/' + name;
                    glp_logv("exporting model %s...", name.c_str());
                    std::fstream output(path, std::ios::out | std::ios::trunc);
                    model->set_directory(name);
                    std::stringstream data = model->serialize_data();
                    auto compressed = glp::util::compress(data.str(), 90);
                    output << compressed;
                }
                {
                    auto name =std::string(buf0) + '/' + std::string(buf1) + ".scene";
                    glp_logv("exporting scene %s...", name.c_str());
                    std::fstream output(name, std::ios::out | std::ios::trunc);
                    std::stringstream data = scene.serialize_data();
                    auto compressed = glp::util::compress(data.str(), 90);
                    output << compressed;
                }
                glp_log("exported!");
            }
            ImGui::TreePop();
        }

        ImGui::RadioButton("PHONG", &is_phong, 1);
        ImGui::SameLine();
        ImGui::RadioButton("PBR", &is_phong, 0);

        if(is_phong) {
            shader = phong;
            shader->bind();
            for(auto& obj: scene.get_objects()) obj->set_shader(shader, shading_t);
            shader->set("fog.color", glm::vec3(fr, fg, fb));
            shader->set("fog.near", fn);
            shader->set("fog.far", ff);
            if(is_directional) {
                scene.get_light().set_type(glp::Object::LightType::DIRECTIONAL);
                shader->set("light.direction", glm::vec3(dirx, diry, dirz));
                shader->set("light.directional", 1);
            } else {
                scene.get_light().set_type(glp::Object::LightType::POINT);
                shader->set("light.position", glm::vec3(posx, posy, posz));
                shader->set("light.directional", 0);
                shader->set("light.linear", linear);
                shader->set("light.quadratic", quadratic);
            }
            shader->set("light.ambient", glm::vec3(ax, ay, az));
            shader->set("light.diffuse", glm::vec3(difx, dify, difz));
            shader->set("light.specular", glm::vec3(sx, sy, sz));
        } else {
            shader = pbr;
            shader->bind();
            for(auto& obj: scene.get_objects()) obj->set_shader(shader, shading_t);
            shader->set("fog.color", glm::vec3(fr, fg, fb));
            shader->set("fog.near", fn);
            shader->set("fog.far", ff);
            if(is_directional) {
                scene.get_light().set_type(glp::Object::LightType::DIRECTIONAL);
                shader->set("light.direction", glm::vec3(dirx, diry, dirz));
                shader->set("light.directional", 1);
            } else {
                scene.get_light().set_type(glp::Object::LightType::POINT);
                shader->set("light.position", glm::vec3(posx, posy, posz));
                shader->set("light.directional", 0);
            }
            shader->set("light.color", glm::vec3(cx, cy, cz));
        }

        ImGui::End();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        sdl.loop_end();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    return 0;
}
