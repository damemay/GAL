#pragma once

#include "placables.hh"
#include "player.hh"
#include "camera.hh"

namespace Object {

class Scene {
    private:
        std::map<std::string, Player*> players_m {};
        std::map<std::string, Camera*> cameras_m {};
        std::map<std::string, Placable*> placables_m {};

        std::string name {};

    public:
        inline Scene(const std::string name_) : name{name_} {}

        inline std::map<std::string, Player*>& players() { return players_m; }
        inline std::map<std::string, Camera*>& cameras() { return cameras_m; }
        inline std::map<std::string, Placable*> placables() { return placables_m; }

        inline void render(Camera* camera) {
            for(const auto& e: placables_m) e.second->render(*camera);
        }

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        inline ~Scene() {
            for(auto& e: placables_m) delete e.second;
        }
};


}
