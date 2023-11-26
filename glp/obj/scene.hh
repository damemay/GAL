#pragma once

#include "LinearMath/btIDebugDraw.h"
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
        inline Scene(size_t width, size_t height, std::vector<SDL_Event>* ev, Shader* shader_) : shader{shader_},
               fog{shader_}, light{LightType::DIRECTIONAL, shader_} {
            camera = new Camera{glm::vec2(width, height), 60.0f};
            player = new PlayerCollFPP{25.0f, camera, ev};
            world = new World{};
            debug_draw = new BulletDebugDraw{};
            world->add_collidable(player);

            debug_draw->setDebugMode(0);
            world->get_bullet_world()->setDebugDrawer(debug_draw);
        }

        inline Scene(size_t width, size_t height, std::vector<SDL_Event>* ev, Shader* shader_, Camera* camera_, bool instantiate_player=true) : shader{shader_},
               camera{camera_}, fog{shader_}, light{LightType::DIRECTIONAL, shader_} {
            if(instantiate_player) player = new PlayerCollFPP{25.0f, camera, ev};
            world = new World{};
            debug_draw = new BulletDebugDraw{};
            if(player) world->add_collidable(player);

            debug_draw->setDebugMode(0);
            world->get_bullet_world()->setDebugDrawer(debug_draw);
        }

        inline Scene(const std::string& path, size_t width, size_t height, std::vector<SDL_Event>* ev, Shader* shader_, ShadingType shading_t) : shader{shader_}, fog{shader_},
               light{LightType::DIRECTIONAL, shader_} {
            auto file = util::read_file(path);
            auto decompressed = util::decompress(file);
            std::stringstream s;
            s << decompressed;

            world = new World{};
            deserialize_data(s, width, height, ev, shading_t, path);
            debug_draw = new BulletDebugDraw{};
        }

        inline void new_object(CollRenderableModel* object) {
            objects.push_back(object);
            world->add_collidable(object);
        }

        inline void update(float dt) {
            if(player) {
                player->fpp_movement_keys();
                player->update();
            }
            world->update(dt);
            world->render(*camera);
        }

        inline void set_debug(bool b) { debug_draw->setDebugMode(b ? btIDebugDraw::DBG_DrawWireframe : 0); }

        inline Camera& get_camera() { return *camera; }
        inline Fog& get_fog() { return fog; }
        inline Light& get_light() { return light; }
        inline PlayerCollFPP* get_player() { return player; }
        inline World* get_world() { return world; }
        inline std::vector<CollRenderableModel*> get_objects() { return objects; }

        std::stringstream serialize_data();

        inline ~Scene() {
            delete world;
            delete debug_draw;
            delete player;
            delete camera;
            for(auto& obj: objects) delete obj;
        }
};

}
}
