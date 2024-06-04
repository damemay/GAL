#include <shader.hh>
#include <gl.hh>

namespace gal {
    namespace render {
        void Shader::compile(const std::string& vertex, const std::string& fragment, bool from_file) { shader = opengl::load_shader(vertex, fragment, false); }
    }
}
