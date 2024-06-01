#pragma once

#include <sdl.hh>
#include <model.hh>
#include <tiny_gltf.h>

namespace gal {
    namespace opengl {
        constexpr GLuint position_attribute_index   = 0;
        constexpr GLuint normal_attribute_index     = 1;
        constexpr GLuint texcoord0_attribute_index  = 2;
        constexpr GLuint joints_attribute_index     = 3;
        constexpr GLuint weights_attribute_index    = 4;

        void load_primitive(render::Primitive* mesh);
        GLuint load_shader(const std::string& vertex, const std::string& fragment, bool from_file=true);
        GLuint load_texture2d(const std::string& texture, bool from_file=true);
        GLuint load_texture2d(const tinygltf::Image& image, const tinygltf::Sampler& sampler);

        void load(SDL_Window* window, SDL_GLContext& context, uint32_t width, uint32_t height);
#ifdef GAL_DEBUG
        void APIENTRY debug(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* user_param);
#endif
    }
}
