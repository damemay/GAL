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
        inline void translate(glm::vec3 translation) { transform = glm::translate(glm::mat4(1.0f), translation); }
        inline void rotate(float rad, glm::vec3 axis) { transform = glm::rotate(transform, glm::radians(rad), axis); }
        inline void scale(glm::vec3 scale) { transform = glm::scale(transform, scale); }

        void load(const std::string& path, Shader* shader, ShadingType shading_type=ShadingType::PBR);
        void render(Camera& camera);

        inline Model* get_model() { return model; }
        inline glm::mat4 get_transform() { return transform; }
        inline void set_transform(const glm::mat4& mat) { transform = mat; }

        void set_shader(Shader* shader_, ShadingType shading_type) {
            shader = shader_;
            model->set_shader(shader);
            model->set_shading_type(shading_type);
        }

        Renderable() {};
        Renderable(const std::string& path, Shader* shader, ShadingType shading_type=ShadingType::PBR);
        Renderable(Model* model, Shader* shader, ShadingType shading_type);
        virtual ~Renderable() {};
};


class Animated: public Renderable {
    private:
        Animation::Animation* find_animation(const std::string& name);
        float* dt {nullptr};
        std::string animation_path {};

    protected:
        Animation::Animator animator;
        std::vector<Animation::Animation*> animations {};

    public:
        void add_animation(const std::string& anim_path);
        void change_animation(const std::string& name);

        inline Animation::Animation* get_animation() { return animator.get_animation(); }
        inline std::string get_path() { return animation_path; }
        void set_path(const std::string& p) { animation_path = p; }

        void render(Camera& camera);

        Animated(const std::string& path, const std::string& anim_path, float* dt_, Shader* shader, ShadingType shading_type);
        Animated(Model* model, const std::string& anim_path, float* dt_, Shader* shader, ShadingType shading_type);
        ~Animated();
};

}

}
