#pragma once

#include "../shader.hh"
#include "../model.hh"
#include "../anim.hh"
#include "camera.hh"

namespace glp {

namespace Object {

class Renderable {
    protected:
        Shader* shader {nullptr};
        Model* model {nullptr};

        glm::mat4 transform {1.0f};

    public: 
        inline void translate(glm::vec3 translation) { transform = glm::translate(transform, translation); }
        inline void rotate(float rad, glm::vec3 axis) { transform = glm::rotate(transform, glm::radians(rad), axis); }
        inline void scale(glm::vec3 scale) { transform = glm::scale(transform, scale); }

        void load(const std::string& path, Shader* shader, ShadingType shading_type=ShadingType::PBR);
        void render(Camera& camera);

        Renderable() {};
        Renderable(const std::string& path, Shader* shader, ShadingType shading_type=ShadingType::PBR);
        Renderable(Model* model, Shader* shader, ShadingType shading_type);
        virtual ~Renderable() {};
};


class Animated: public Renderable {
    private:
        Animation::Animation* find_animation(const std::string& name);
        float* dt {nullptr};

    protected:
        Animation::Animator animator;
        std::vector<Animation::Animation*> animations {};

    public:
        void add_animation(const std::string& anim_path);
        void change_animation(const std::string& name);

        void render(Camera& camera);

        Animated(const std::string& path, const std::string& anim_path, float* dt_, Shader* shader, ShadingType shading_type);
        ~Animated();
};

}

}
