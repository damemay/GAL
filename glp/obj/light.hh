#pragma once

#include <shader.hh>
#include <material.hh>

namespace glp {

namespace Object {

class Fog {
    private:
        Shader* shader;
        
        glm::vec3 color {0.4f, 0.4f, 0.4f};
        float near {0.1f};
        float far {100.0f};
    public:
        inline void set_color(glm::vec3 v) {
            color = v;
            shader->bind();
            shader->set("fog.color", color);
        }
        inline void set_near(float v) {
            near = v;
            shader->bind();
            shader->set("fog.near", near);
        }
        inline void set_far(float v) {
            far = v;
            shader->bind();
            shader->set("fog.far", far);
        }

        inline glm::vec3 get_color() { return color; }
        inline float get_near() { return near; }
        inline float get_far() { return far; }

        Fog(Shader* s) : shader{s} {
            shader->bind();
            shader->set("fog.color", color);
            shader->set("fog.near", near);
            shader->set("fog.far", far);
        };
        ~Fog() = default;
};

enum class LightType {
    DIRECTIONAL,
    POINT,
};

class Light {
    private:
        LightType type;

        Shader* shader;
        
        glm::vec3 position {1.2f, 1.0f, 2.0f};
        glm::vec3 direction {-0.2f, -1.0f, -0.3f};

        glm::vec3 ambient {0.2f, 0.2f, 0.2f};
        glm::vec3 diffuse {0.5f, 0.5f, 0.5f};
        glm::vec3 specular{0.7f, 0.7f, 0.7f};
        glm::vec3 color {300, 300, 300};

        float linear {0.09f};
        float quadratic {0.032f};

    public:
        inline void set_type(LightType t) {
            type = t;
            shader->bind();
            shader->set("light.directional", type == LightType::DIRECTIONAL ? 1 : 0);
        }

        inline void set_shader(Shader* s) { shader = s; }

        inline void set_position(glm::vec3 v) {
            position = v;
            shader->bind();
            shader->set("light.position", position);
        }
        inline void set_direction(glm::vec3 v) {
            direction = v;
            shader->bind();
            shader->set("light.direction", direction);
        }
        inline void set_ambient(glm::vec3 v) {
            ambient = v;
            shader->bind();
            shader->set("light.ambient", ambient);
        }
        inline void set_diffuse(glm::vec3 v) {
            diffuse = v;
            shader->bind();
            shader->set("light.diffuse", diffuse);
        }
        inline void set_specular(glm::vec3 v) {
            specular = v;
            shader->bind();
            shader->set("light.specular", specular);
        }
        inline void set_color(glm::vec3 v) {
            color = v;
            shader->bind();
            shader->set("light.color", color);
        }
        inline void set_linear(float v) {
            linear = v;
            shader->bind();
            shader->set("light.linear", linear);
        }
        inline void set_quadratic(float v) {
            quadratic = v;
            shader->bind();
            shader->set("light.quadratic", quadratic);
        }

        inline LightType get_type() { return type; }
        inline glm::vec3 get_position() { return position; }
        inline glm::vec3 get_direction() { return direction; }
        inline glm::vec3 get_ambient() { return ambient; }
        inline glm::vec3 get_diffuse() { return diffuse; }
        inline glm::vec3 get_specular() { return specular; }
        inline float get_linear() { return linear; }
        inline float get_quadratic() { return quadratic; }

        Light(LightType t, Shader* s) : shader{s} {
            set_type(t);
            shader->bind();
            shader->set("light.directional", t == LightType::DIRECTIONAL ? 1 : 0);
            shader->set("light.position", position);
            shader->set("light.direction", direction);
            shader->set("light.ambient", ambient);
            shader->set("light.diffuse", diffuse);
            shader->set("light.specular", specular);
            shader->set("light.color", color);
            shader->set("light.linear", linear);
            shader->set("light.quadratic", quadratic);
        }
        ~Light() = default;
};

}

}
