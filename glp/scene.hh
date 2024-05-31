#pragma once

#include <glp.hh>

namespace glp {
    class Scene {
        glm::vec2 screen_dimensions_;
        const GLP& glp_reference_;

        public:
            Scene(const GLP& glp);

            void loop();
    };
}
