#include <model.hh>
#include <utils.hh>
#include <gltf.hh>
#include <gl.hh>

namespace glp {
    namespace render {
        Primitive::Primitive(const std::vector<Vertex>& vert, const std::vector<unsigned int>& idx) : vertices{vert}, indices{idx} {
            opengl::load_primitive(this);
        }
    }

    GLTF_Render_Model::GLTF_Render_Model(const std::string& path) {
#ifdef GLP_DEBUG
        util::print(std::format("loading {}...", path));
#endif
        auto tinygltf_model = gltf::Model(path);
        primitives = tinygltf_model.primitives;
#ifdef GLP_DEBUG
        util::print(std::format("generating shaders for {}...", path));
#endif
        for(auto& [prim, mat]: primitives) mat.generate_shader();
    }

    GLTF_Render_Model::~GLTF_Render_Model() {
        for(auto& [prim, mat]: primitives) {
            glDeleteVertexArrays(1, &prim.vao);
            glDeleteBuffers(1, &prim.vbo);
            glDeleteBuffers(1, &prim.ebo);
            for(auto& [id, tex]: mat.textures) {
                glDeleteTextures(1, &tex);
            }
            glDeleteProgram(mat.shader);
        }
    }
}
