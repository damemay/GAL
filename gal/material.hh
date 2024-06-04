#pragma once

#include <map>
#include <sdl.hh>
#include <shader.hh>

namespace gal {
    namespace render {
        struct Material : Shader {
            std::map<uint8_t, GLuint> textures;
            std::map<std::string, GLint> uniforms;

            glm::vec4 albedo {};
            float metallic {};
            float roughness {};

            int8_t albedo_id {-1};
            int8_t metallic_id {-1};
            int8_t roughness_id {-1};
            int8_t occlusion_id {-1};
            int8_t normal_id {-1};

            void setup_uniforms();
        };
    }
}
