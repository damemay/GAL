#pragma once

#include "../shader.hh"
#include "../model.hh"
#include "../anim.hh"
#include "camera.hh"

namespace glp {

namespace Object {

class Placable {
    public: 
        virtual void render(Camera& camera) = 0;
        virtual ~Placable() {}
};

class Static : public Placable {
    protected:
        Shader* shader {nullptr};
        Model* model {nullptr};

        glm::mat4 transform {1.0f};

        glm::mat4 mvp(Camera& camera);

    public:
        inline void translate(glm::vec3 translation) { transform = glm::translate(transform, translation); }
        inline void rotate(float rad, glm::vec3 axis) { transform = glm::rotate(transform, glm::radians(rad), axis); }
        inline void scale(glm::vec3 scale) { transform = glm::scale(transform, scale); }

        void render(Camera& camera);

        Static() {}
        Static(const std::string& path);
        ~Static();
};



class Animated: public Static {
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

        Animated(const std::string& path, const std::string& anim_path, float* dt);
        ~Animated();
};

}

}
