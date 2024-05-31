#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <sdl.hh>
#include <scene.hh>

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
        };
        
        struct Mesh {
            std::vector<Primitive> primitives_ {};
        
            Mesh(const std::string& path);
            ~Mesh();
        
            void draw();
        };
    }

    struct Model: scene::Renderable, render::Mesh {
        Model(const std::string& path);
        ~Model() = default;
        void render();
    };
}
