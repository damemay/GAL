#pragma once

#include "LinearMath/btIDebugDraw.h"
#include "material.hh"
#include "utils.hh"
#include <obj/camera.hh>
#include <obj/player.hh>
#include <obj/light.hh>
#include <obj/collidable.hh>
#include <obj/renderable.hh>
#include <sstream>

namespace glp {

namespace Object {

class Scene {
    protected:
        Shader* shader {nullptr};
        Camera* camera {nullptr};
        Fog fog;
        Light light;

        PlayerFPP* player {nullptr};
        std::vector<Renderable*> objects;

        void deserialize_data(std::stringstream& s, size_t width, size_t height, std::vector<SDL_Event>* ev, ShadingType shading_t, float* dt, const std::string& path);

    public:
        Scene(size_t width, size_t height, std::vector<SDL_Event>* ev, Shader* shader_, bool instantiate_player=true);
        Scene(size_t width, size_t height, std::vector<SDL_Event>* ev, Shader* shader_, Camera* camera_, bool instantiate_player=true);
        Scene(const std::string& path, size_t width, size_t height, std::vector<SDL_Event>* ev, float* dt, Shader* shader_, Camera* camera_, ShadingType shading_t);

        void new_object(Renderable* object);
        void update(float dt);

        inline Camera& get_camera() { return *camera; }
        inline Fog& get_fog() { return fog; }
        inline Light& get_light() { return light; }
        inline PlayerFPP* get_player() { return player; }
        inline std::vector<Renderable*> get_objects() { return objects; }

        std::stringstream serialize_data();

        ~Scene();
};

class PhysicsScene {
    private:
        Shader* shader {nullptr};

        Camera* camera {nullptr};
        Fog fog;
        Light light;

        PlayerCollFPP* player {nullptr};

        World* world {nullptr};

        BulletDebugDraw* debug_draw {nullptr};

        std::vector<CollRenderableModel*> objects;

        void deserialize_data(std::stringstream& s, size_t width, size_t height, std::vector<SDL_Event>* ev, ShadingType shading_t, const std::string& path);

    public:
        PhysicsScene(size_t width, size_t height, std::vector<SDL_Event>* ev, Shader* shader_);
        PhysicsScene(size_t width, size_t height, std::vector<SDL_Event>* ev, Shader* shader_, Camera* camera_, bool instantiate_player=true);
        PhysicsScene(const std::string& path, size_t width, size_t height, std::vector<SDL_Event>* ev, Shader* shader_, ShadingType shading_t);

        void new_object(CollRenderableModel* object);

        void update(float dt);

        inline void set_debug(bool b) { debug_draw->setDebugMode(b ? btIDebugDraw::DBG_DrawWireframe : 0); }

        inline Camera& get_camera() { return *camera; }
        inline Fog& get_fog() { return fog; }
        inline Light& get_light() { return light; }
        inline PlayerCollFPP* get_player() { return player; }
        inline World* get_world() { return world; }
        inline std::vector<CollRenderableModel*> get_objects() { return objects; }

        std::stringstream serialize_data();

        ~PhysicsScene();
};

}
}
