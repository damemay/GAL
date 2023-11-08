#pragma once

#include "placables.hh"
#include "player.hh"
#include "camera.hh"

namespace Object {

class Scene {
    private:
        Player player {};
        Camera camera {};
        std::map<std::string, Placable*> placables {};

        std::string name {};

    public:
        inline Scene(const std::string name_) : name{name_} {}

        inline Player& get_player() { return player; }
        inline Camera& get_camera() { return camera; }
        inline std::map<std::string, Placable*> get_placables() { return placables; }

        inline void render() {
            for(const auto& e: placables) e.second->render(camera);
        }

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        inline ~Scene() {
            for(auto& e: placables) delete e.second;
        }
};


}
