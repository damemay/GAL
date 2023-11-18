#include "obj/renderable.hh"

namespace glp {

namespace Object {

void Renderable::render(Camera& camera) {
    if(shader) {
        shader->bind();
        shader->set("vp", camera.view_projection());
        shader->set("model", transform);
        shader->set("camera_position", camera.get_position());
        model->render();
        shader->unbind();
    }
}

void Renderable::load(const std::string& path, Shader* shader_, ShadingType shading_type) {
    model = new Model(path, shader_, shading_type);
    shader = shader_;
}

Renderable::Renderable(const std::string& path, Shader* shader, ShadingType shading_type) {
    load(path, shader, shading_type);
}

Renderable::Renderable(Model* m, Shader* s, ShadingType shading_type) : model{m}, shader{s} {
    model->set_shader(s);
    model->set_shading_type(shading_type);
}

Animation::Animation* Animated::find_animation(const std::string& name) {
    for(auto& anim: animations)
        if(anim->get_name() == name)
            return anim;
    return nullptr;
}

void Animated::change_animation(const std::string& name) {
    auto anim = find_animation(name);
    if(anim) animator.play_animation(anim);
    return;
}

void Animated::add_animation(const std::string& anim_path) {
    auto anim = new Animation::Animation(anim_path, *model);
    animations.push_back(anim);
}

void Animated::render(Camera& camera) {
    shader->bind();
    shader->set("vp", camera.view_projection());
    shader->set("model", transform);
    shader->set("camera_position", camera.get_position());
    animator.update(*dt);
    model->render();
    shader->unbind();
}

Animated::Animated(const std::string& path, const std::string& anim_path, float* dt_, Shader* shader_, ShadingType shading_type)
    : dt{dt_} {
    model = new Model(path, shader_, shading_type);
    shader = shader_;
    
    auto anim = new Animation::Animation(anim_path, *model);
    animations.push_back(anim);
    
    animator = Animation::Animator(anim, model);
}

Animated::~Animated() {
    for(auto& anim: animations) delete anim;
}

}

}
