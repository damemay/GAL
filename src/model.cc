#include <model.hh>
#include <util.hh>
#include <gltf.hh>
#include <gl.hh>

namespace gal {
    namespace render {
        Primitive::Primitive(const std::vector<Vertex>& vert, const std::vector<unsigned int>& idx) : vertices{vert}, indices{idx} {
            opengl::load_primitive(this);
        }
    }

    GLTF_Render_Model::GLTF_Render_Model(const std::string& path) {
#ifdef GAL_DEBUG
        util::print(std::format("loading {}...", path));
#endif
        auto tinygltf_model = gltf::Model(path);
        primitives = tinygltf_model.primitives;
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
