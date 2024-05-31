#pragma once

#include <functional>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sdl.hh>
#include <scene.hh>
#include <material.hh>

namespace glp {
    namespace render {
        constexpr GLuint MAX_BONE_INFLUENCE             = 4;
        constexpr GLuint MAX_BONES                      = 100;
        
        struct Vertex {
            glm::vec3 position;
            glm::vec3 normal;
            glm::vec2 uv;
        
            float bone_index[MAX_BONE_INFLUENCE];
            float weights[MAX_BONE_INFLUENCE];
        };
        
        struct Bone_Info {
            std::string name;
            glm::mat4 offset;
        };
        
        struct Primitive {
            GLuint vao, vbo, ebo;
                
            std::vector<Vertex>             vertices;
            std::vector<unsigned int>       indices;
        
            Primitive(const std::vector<Vertex>& vert, const std::vector<unsigned int>& idx);
            bool operator==(const Primitive& o) const {
                return (vao == o.vao && vbo == o.vbo && o.ebo);
            }
        };
    }
}

template <>
struct std::hash<glp::render::Primitive> {
    std::size_t operator()(const glp::render::Primitive& k) const {
        return ((std::hash<unsigned int>()(k.vao)
                 ^ (std::hash<unsigned int>()(k.vbo) << 1)) >> 1)
                 ^ (std::hash<unsigned int>()(k.ebo) << 1);
      }
};

namespace glp {
    namespace render {
        struct Mesh {
            std::unordered_map<Primitive, Material> primitives;
        
            Mesh(const std::string& path);
            ~Mesh();
        
            void draw();
        };
    }

    struct Render_Model: scene::Renderable, render::Mesh {
        inline Render_Model(const std::string& path) : render::Mesh{path} {}
        ~Render_Model() = default;
        inline void render() { draw(); }
    };
}

