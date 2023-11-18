#pragma once

#include "LinearMath/btIDebugDraw.h"
#include <obj/camera.hh>
#include <obj/player.hh>
#include <obj/light.hh>
#include <obj/collidable.hh>
#include <obj/renderable.hh>

namespace glp {

namespace Object {

class Scene {
    private:
        Shader* shader;

        Camera camera;
        Fog fog;
        Light light;

        PlayerCollFPP* player;

        World* world;

        BulletDebugDraw* debug_draw;

        std::vector<CollRenderableModel*> objects;

    public:
        inline Scene(size_t width, size_t height, std::vector<SDL_Event>* ev, Shader* shader_) : shader{shader_},
               camera{glm::vec2(width, height), 60.0f}, fog{shader_},
               light{LightType::DIRECTIONAL, shader_} {
            player = new PlayerCollFPP{25.0f, &camera, ev};
            world = new World{};
            debug_draw = new BulletDebugDraw{};
            world->add_collidable(player);

            debug_draw->setDebugMode(0);
            world->get_bullet_world()->setDebugDrawer(debug_draw);
        }

        inline void new_object(CollRenderableModel* object) {
            objects.push_back(object);
            world->add_collidable(object);
        }

        inline void update(float dt) {
            player->fpp_movement_keys();
            player->update();
            world->update(dt);
            world->render(camera);
        }

        inline void set_debug(bool b) { debug_draw->setDebugMode(b ? btIDebugDraw::DBG_DrawWireframe : 0); }

        inline Camera& get_camera() { return camera; }
        inline Fog& get_fog() { return fog; }
        inline Light& get_light() { return light; }
        inline PlayerCollFPP* get_player() { return player; }
        inline World* get_world() { return world; }

        inline ~Scene() {
            delete world;
            delete debug_draw;
            delete player;
            for(auto& obj: objects) delete obj;
        }
};

}
}
