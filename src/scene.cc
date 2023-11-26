#include "obj/collidable.hh"
#include "obj/light.hh"
#include <obj/scene.hh>
#include <sstream>
#include <map>

namespace glp {

namespace Object {

std::stringstream Scene::serialize_data() {
    std::stringstream s;
    s << "cam " << camera->get_fov() << ' '
        << camera->get_near() << ' '
        << camera->get_far() << ' '
        << camera->get_position().x << ' '
        << camera->get_position().y << ' '
        << camera->get_position().z << ' ';
    if(player) {
        s << "pl " << player->get_speed() << ' ';
    } else {
        s << "pl " << 25 << ' ';
    }
    s << "fog " << fog.get_color().x << ' '
        << fog.get_color().y << ' '
        << fog.get_color().z << ' '
        << fog.get_near() << ' '
        << fog.get_far() << ' ';
    s << "lt " << (light.get_type()==LightType::DIRECTIONAL ? "dir" : "pt") << ' ';
    s << light.get_position().x << ' '
        << light.get_position().y << ' '
        << light.get_position().z << ' '
        << light.get_direction().x << ' '
        << light.get_direction().y << ' '
        << light.get_direction().z << ' '
        << light.get_ambient().x << ' '
        << light.get_ambient().y << ' '
        << light.get_ambient().z << ' '
        << light.get_diffuse().x << ' '
        << light.get_diffuse().y << ' '
        << light.get_diffuse().z << ' '
        << light.get_specular().x << ' '
        << light.get_specular().y << ' '
        << light.get_specular().z << ' '
        << light.get_linear() << ' ' << light.get_quadratic() << ' ';
    s << "objs " << objects.size() << ' ';

    for(auto& obj : objects) {
        s << "obj ";
        s << (obj->get_model()->get_directory().empty() ? "null" : obj->get_model()->get_directory()) << ' ';
        auto t = obj->get_rigidbody()->getWorldTransform();
        auto pos = t.getOrigin();
        auto rot = t.getRotation();
        s << pos.getX() << ' '
            << pos.getY() << ' '
            << pos.getZ() << ' '
            << rot.getX() << ' '
            << rot.getY() << ' '
            << rot.getZ() << ' '
            << rot.getW() << ' ';
    }

    return s;
}

void Scene::deserialize_data(std::stringstream& s, size_t width, size_t height, std::vector<SDL_Event>* ev, ShadingType shading_t, const std::string& path) {
    auto scene_path = path.substr(0, path.find_last_of('/')) + '/';
    glp_logv("scene path: %s", scene_path.c_str());
    std::string name;
    size_t count;

    s >> name; assert(name=="cam");
    {
        float posx, posy, posz, fov, near, far;
        s >> fov >> near >> far >> posx >> posy >> posz;
        camera = new Camera{glm::vec3(posx, posy, posz), glm::vec2(width, height), fov, near, far};
    }
    s >> name; assert(name=="pl");
    {
        float spd;
        s >> spd;
        player = new PlayerCollFPP{spd, camera, ev};
        world->add_collidable(player);
    }
    s >> name; assert(name=="fog");
    {
        float r, g, b, n, f;
        s >> r >> g >> b >> n >> f;
        fog.set_color(glm::vec3(r, g, b));
        fog.set_far(f);
        fog.set_near(n);
    }
    s >> name; assert(name=="lt");
    {
        s >> name;
        light.set_type(name=="dir" ? LightType::DIRECTIONAL : LightType::POINT);
        float posx, posy, posz, dirx, diry, dirz,
              ar, ag, ab, dr, dg, db, sr, sg, sb, l, q;
        s >> posx >> posy >> posz >> dirx >> diry >> dirz
            >> ar >> ag >> ab >> dr >> dg >> db >> sr >> sg
            >> sb >> l >> q;
        light.set_position(glm::vec3(posx, posy, posz));
        light.set_direction(glm::vec3(dirx, diry, dirz));
        light.set_ambient(glm::vec3(ar, ag, ab));
        light.set_diffuse(glm::vec3(dr,dg,db));
        light.set_specular(glm::vec3(sr,sg,sb));
        light.set_linear(l);
        light.set_quadratic(q);
    }
    s >> name; assert(name=="objs");
    s >> count;
    objects.resize(count);
    std::map<std::string, Model*> dirs;
    for(size_t i=0; i<count; i++) {
        s >> name; assert(name=="obj");
        s >> name; if(name=="null") continue;
        Model* mod{nullptr};
        for(auto& dir: dirs) if(dir.first==name) mod = dir.second;
        if(!mod) {
            mod = new Model{scene_path+name, shader, shading_t};
            dirs.emplace(name, mod);
        }
        float posx, posy, posz, rotx, roty, rotz, rotw;
        s >> posx >> posy >> posz >> rotx >> roty >> rotz >> rotw;
        new_object(new CollRenderableModel{mod, shader, shading_t, 0.0f, btVector3(posx,posy,posz), btQuaternion(rotx,roty,rotz,rotw)});

    }
}

}

}
