#include <model.hh>
#include <utils.hh>
#include <gl.hh>
#include <gltf.hh>

namespace glp {
    namespace render {
        Primitive::Primitive(const std::vector<Vertex>& vert, const std::vector<unsigned int>& idx) : vertices{vert}, indices{idx} {
            opengl::load_primitive(this);
        }

        Mesh::Mesh(const std::string& path) {
            auto tinygltf_model = gltf::load_model(path);
            primitives_ = gltf::setup_primitives(tinygltf_model);
        }
    }
}
