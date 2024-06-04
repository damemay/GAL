#pragma once

#include <glm/glm.hpp>
#include <material.hh>

namespace gal {
    namespace render {
        struct Fog {
            glm::vec3 color {0.4f};
            float near {0.1f};
            float far {100.0f};

            Fog() {}
            ~Fog() = default;

            void set(const Material& material);
        };

        struct Light {
            glm::vec3 position {1.2f, 1.0f, 2.0f};
            glm::vec3 color {300};

            void set(const Material& material);
        };

        struct Directional_Light : Light {
            glm::vec3 direction {-0.2f, -1.0f, -0.3f};

            Directional_Light() {}
            ~Directional_Light() = default;

            void set(const Material& material);
        };


        constexpr uint16_t shadow_wh = 1024;
        constexpr auto shadow_vertex_shader =
            "#version 410 core\n"
            "layout (location = 0) in vec3 pos;\n"
            "uniform mat4 light_space_mat;\n"
            "uniform mat4 model;\n"
            "void main() {\n"
            "gl_Position = light_space_mat * model * vec4(pos, 1.0);\n"
            "}\n";
        constexpr auto shader_fragment_shader =
            "#version 410 core\n"
            "void main() {}\n";
        struct Shadow_Map {
            GLuint framebuffer;
            GLuint texture;
            Shader shader;

            float near {1.0f};
            float far {7.5f};
            glm::vec4 frustrum {-10.0f, 10.0f, -10.0f, 10.0f};
            glm::mat4 projection {glm::ortho(frustrum.x, frustrum.y, frustrum.z, frustrum.w, near, far)};
            glm::mat4 light_space_matrix {};

            Shadow_Map();
        };

        template <typename T>
        class Environment {
            T light_;
            Fog fog_;

            Shadow_Map shadow_map_ {};

            public:
                Environment() {}

                ~Environment() {

                }

                void use_depth_map() {
                    glm::mat4 light_view = glm::lookAt(light_.position, light_.direction, glm::vec3(0.0f, 1.0f, 0.0f));
                    shadow_map_.light_space_matrix = shadow_map_.projection * light_view;
                    shadow_map_.shader.use();
                    shadow_map_.shader.set("light_space_mat", shadow_map_.light_space_matrix);
                    glViewport(0, 0, shadow_wh, shadow_wh);
                    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_.framebuffer);
                    glClear(GL_DEPTH_BUFFER_BIT);
                    glActiveTexture(GL_TEXTURE0);
                    glBindTexture(GL_TEXTURE_2D, shadow_map_.texture);
                }

                glm::mat4 light_space_matrix() { return shadow_map_.light_space_matrix; }
                GLuint depth_map_texture() { return shadow_map_.texture; }
                const Shader& depth_map_shader() { return shadow_map_.shader; }
                T* light() { return &light_; }
                Fog* fog() { return &fog_; }
        };
    }
}
