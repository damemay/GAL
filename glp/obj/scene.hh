#pragma once

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

        std::vector<CollRenderableModel*> objects;

    public:
        inline Scene(size_t width, size_t height, std::vector<SDL_Event>* ev, Shader* shader_) : shader{shader_},
               camera{glm::vec2(width, height), 60.0f}, fog{shader_},
               light{LightType::DIRECTIONAL, shader_} {
            player = new PlayerCollFPP{25.0f, &camera, ev};
            world = new World{};
            world->add_collidable(player);
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

        inline Camera& get_camera() { return camera; }
        inline Fog& get_fog() { return fog; }
        inline Light& get_light() { return light; }
        inline PlayerCollFPP* get_player() { return player; }
        inline World* get_world() { return world; }

        inline ~Scene() {
            delete world;
            delete player;
            for(auto& obj: objects) delete obj;
        }
};

}
}
