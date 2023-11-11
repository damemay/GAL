#include <SDL2/SDL.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl2.h"
#include "imgui/imgui_impl_opengl3.h"
#include "../../glp/utils.hh"
#include "../../glp/obj/camera.hh"
#include "../../glp/obj/player.hh"
#include "../../glp/obj/placables.hh"

static std::vector<Object::Static*> statics;
static std::vector<Object::Animated*> animated;
uint64_t start_tick {0}, last_tick {0};
float dt {0.0f};

int main(int argc, char* argv[]) {
    if(SDL_Init(SDL_INIT_VIDEO) > 0) {
        glp_diev("could not init SDL: %s", SDL_GetError());
    }
    atexit(SDL_Quit);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS,
    SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
    SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_Window* window = nullptr;
    if(!(window = SDL_CreateWindow("glp-scener",
                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                    1280, 720, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE)))
        glp_diev("could not create SDL Window: %s", SDL_GetError());
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if(!context)
        glp_diev("could not create SDL_GLContext: %s", SDL_GetError());
    SDL_GL_MakeCurrent(window, context);
    if(!gladLoadGLLoader(SDL_GL_GetProcAddress))
        glp_die("could not load glad!");
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    
    ImGui_ImplSDL2_InitForOpenGL(window, context);
    ImGui_ImplOpenGL3_Init();

    Object::Camera camera {glm::vec2(1280.0f, 720.0f), 90.0f};

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    bool run = true;
    while(run) {
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if(event.type == SDL_QUIT) run = false;
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        glClearColor(0.4, 0.4, 0.4, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        start_tick = SDL_GetTicks64();
        dt = (start_tick - last_tick) / 1000.0f;

        for(auto& e: statics) {
            e->render(camera);
        }
        for(auto& e: animated) {
            e->render(camera);
        }

        ImGui::SetNextWindowSize(ImVec2(250, 680));
        ImGui::Begin("glp-scener");
        if(ImGui::TreeNode("Scene")) {
            if(ImGui::TreeNode("Camera")) {
                if(ImGui::TreeNode("Position")) {
                    static float cam_x = 0.000f;
                    static float cam_y = 0.000f;
                    static float cam_z = 0.000f;
                    ImGui::InputFloat("X", &cam_x, 1.0f);
                    ImGui::InputFloat("Y", &cam_y, 1.0f);
                    ImGui::InputFloat("Z", &cam_z, 1.0f);
                    camera.set_position(glm::vec3(cam_x, cam_y, cam_z));
                    ImGui::TreePop();
                }
                if(ImGui::TreeNode("Rotation")) {
                    static float yaw = -90.0f;
                    static float pitch = 0.0f;
                    ImGui::InputFloat("Yaw", &yaw, 1.0f);
                    ImGui::InputFloat("Pitch", &pitch, 1.0f);
                    camera.set_yaw(yaw);
                    camera.set_pitch(pitch);
                    ImGui::TreePop();
                }
                static float fov = 90.0f;
                ImGui::InputFloat("FOV", &fov, 1.0f);
                camera.set_fov(fov);
                ImGui::TreePop();
            }
            for(auto& e: statics) {
                if(ImGui::TreeNode("Static Object")) {
                    if(ImGui::TreeNode("Position")) {
                        static float x = 0.0f;
                        static float y = 0.0f;
                        static float z = 0.0f;
                        if(ImGui::InputFloat("X", &x, 1.0f))
                            e->translate(glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Y", &y, 1.0f))
                            e->translate(glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Z", &z, 1.0f))
                            e->translate(glm::vec3(x,y,z));
                        ImGui::TreePop();
                    }
                    if(ImGui::TreeNode("Rotation")) {
                        static float rot = 0.0f;
                        static float x = 0.0f;
                        static float y = 0.0f;
                        static float z = 0.0f;
                        if(ImGui::InputFloat("RAD", &rot, 1.0f))
                            e->rotate(rot, glm::vec3(x,y,z));
                        if(ImGui::InputFloat("X", &x, 1.0f))
                            e->rotate(rot, glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Y", &y, 1.0f))
                            e->rotate(rot, glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Z", &z, 1.0f))
                            e->rotate(rot, glm::vec3(x,y,z));
                        ImGui::TreePop();
                    }
                    if(ImGui::TreeNode("Scale")) {
                        static float x = 1.0f;
                        static float y = 1.0f;
                        static float z = 1.0f;
                        if(ImGui::InputFloat("X", &x, 1.0f))
                            e->scale(glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Y", &y, 1.0f))
                            e->scale(glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Z", &z, 1.0f))
                            e->scale(glm::vec3(x,y,z));
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            for(auto& e: animated) {
                if(ImGui::TreeNode("Animated Object")) {
                    if(ImGui::TreeNode("Position")) {
                        static float x = 0.0f;
                        static float y = 0.0f;
                        static float z = 0.0f;
                        if(ImGui::InputFloat("X", &x, 1.0f))
                            e->translate(glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Y", &y, 1.0f))
                            e->translate(glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Z", &z, 1.0f))
                            e->translate(glm::vec3(x,y,z));
                        ImGui::TreePop();
                    }
                    if(ImGui::TreeNode("Rotation")) {
                        static float rot = 0.0f;
                        static float x = 0.0f;
                        static float y = 0.0f;
                        static float z = 0.0f;
                        if(ImGui::InputFloat("RAD", &rot, 1.0f))
                            e->rotate(rot, glm::vec3(x,y,z));
                        if(ImGui::InputFloat("X", &x, 1.0f))
                            e->rotate(rot, glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Y", &y, 1.0f))
                            e->rotate(rot, glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Z", &z, 1.0f))
                            e->rotate(rot, glm::vec3(x,y,z));
                        ImGui::TreePop();
                    }
                    if(ImGui::TreeNode("Scale")) {
                        static float x = 1.0f;
                        static float y = 1.0f;
                        static float z = 1.0f;
                        if(ImGui::InputFloat("X", &x, 1.0f))
                            e->scale(glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Y", &y, 1.0f))
                            e->scale(glm::vec3(x,y,z));
                        if(ImGui::InputFloat("Z", &z, 1.0f))
                            e->scale(glm::vec3(x,y,z));
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }

        if(ImGui::TreeNode("Add static model")) {
            static char buf[2048];
            ImGui::InputText("path", buf, IM_ARRAYSIZE(buf));
            if(ImGui::Button("Add model")) {
                glp_logv("%s", buf);
                auto t = new Object::Static{buf};
                statics.push_back(t);
            }
            ImGui::TreePop();
        }
        if(ImGui::TreeNode("Add anim model")) {
            static char buf[2048];
            static char buf1[2048];
            ImGui::InputText("model", buf, IM_ARRAYSIZE(buf));
            ImGui::InputText("anim", buf1, IM_ARRAYSIZE(buf1));
            if(ImGui::Button("Add model")) {
                glp_logv("%s %s", buf, buf1);
                auto t = new Object::Animated{buf, buf1, &dt};
                animated.push_back(t);
            }
            ImGui::TreePop();
        }

        ImGui::End();

        last_tick = start_tick;

        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);

    return 0;
}
