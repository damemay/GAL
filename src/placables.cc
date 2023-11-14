#include "placables.hh"

namespace Object {

glm::mat4 Static::mvp(Camera& camera) {
    auto vp = camera.view_projection();
    auto mvp = vp.second * vp.first * transform;
    return mvp;
}

void Static::render(Camera& camera) {
    shader->bind();
    auto m = mvp(camera);
    shader->set("mvp", m);
    model->render();
}

Static::Static(const std::string& path) {
#ifndef __vita__
    shader = new Shader("../res/shaders/static.vert", "../res/shaders/textured.frag");
#else
    shader = new Shader("../res/shaders/vita/vita_static.vert", "../res/shaders/vita/vita_textured.frag");
#endif
    model = new Model(path, shader);
}

Static::~Static() {
    delete model;
    delete shader;
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
    auto m = mvp(camera);
    shader->set("mvp", m);
    animator.update(*dt);
    model->render();
}

Animated::Animated(const std::string& path, const std::string& anim_path, float* dt_)
    : dt{dt_} {
#ifndef __vita__
    shader = new Shader("../res/shaders/skinned.vert", "../res/shaders/textured.frag");
#else 
    shader = new Shader("../res/shaders/vita/vita_skinned.vert", "../res/shaders/vita/vita_textured.frag");
#endif
    model = new Model(path, shader);
    
    auto anim = new Animation::Animation(anim_path, *model);
    animations.push_back(anim);
    
    animator = Animation::Animator(anim, model);
}

Animated::~Animated() {
    for(auto& anim: animations) delete anim;
}

}
