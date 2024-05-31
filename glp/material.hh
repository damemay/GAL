#pragma once

#include <map>
#include <sdl.hh>

namespace glp {
    namespace render {
        struct Material {
            GLuint shader;
            std::map<uint8_t, GLuint> textures;

            glm::vec4 albedo {};
            float metallic {};
            float roughness {};

            int8_t albedo_id {-1};
            int8_t metallic_id {-1};
            int8_t roughness_id {-1};
            int8_t occlusion_id {-1};
            int8_t normal_id {-1};

            void generate_shader();
        };
    }
}
