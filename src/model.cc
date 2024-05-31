#include <model.hh>
#include <utils.hh>
#include <gltf.hh>
#include <gl.hh>

namespace glp {
    namespace render {
        Primitive::Primitive(const std::vector<Vertex>& vert, const std::vector<unsigned int>& idx) : vertices{vert}, indices{idx} {
            opengl::load_primitive(this);
        }

        Mesh::Mesh(const std::string& path) {
            auto tinygltf_model = gltf::Model(path);
            primitives = tinygltf_model.primitives;
            for(auto& [prim, mat]: primitives) mat.generate_shader();
        }

        Mesh::~Mesh() {
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

        void Mesh::draw() {
            for(auto& [prim, mat]: primitives) {
                glUseProgram(mat.shader);
                glBindVertexArray(prim.vao);
                glDrawElements(GL_TRIANGLES, prim.indices.size(), GL_UNSIGNED_INT, nullptr);
                glBindVertexArray(0);
                glUseProgram(0);
            }
        }
    }
}
