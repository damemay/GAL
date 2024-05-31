#include <gl.hh>
#include <utils.hh>
#include <stb_image.h>

#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <format>

namespace glp {
    namespace opengl {
        void load_primitive(render::Primitive* mesh) {
            glGenVertexArrays(1, &mesh->vao);
            glGenBuffers(1, &mesh->vbo);
            glGenBuffers(1, &mesh->ebo);

            glBindVertexArray(mesh->vao);

            glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
            glBufferData(GL_ARRAY_BUFFER, mesh->vertices.size() * sizeof(render::Vertex), mesh->vertices.data(), GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices.size() * sizeof(unsigned int), mesh->indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(position_attribute_index, 3, GL_FLOAT, GL_FALSE, sizeof(render::Vertex), nullptr);
            glEnableVertexAttribArray(position_attribute_index);

            glVertexAttribPointer(normal_attribute_index, 3, GL_FLOAT, GL_FALSE, sizeof(render::Vertex), (void*)offsetof(render::Vertex, normal));
            glEnableVertexAttribArray(normal_attribute_index);

            glVertexAttribPointer(texcoord0_attribute_index, 2, GL_FLOAT, GL_FALSE, sizeof(render::Vertex), (void*)offsetof(render::Vertex, uv));
            glEnableVertexAttribArray(texcoord0_attribute_index);

            glVertexAttribPointer(joints_attribute_index, 4, GL_FLOAT, GL_FALSE, sizeof(render::Vertex), (void*)offsetof(render::Vertex, bone_index));
            glEnableVertexAttribArray(joints_attribute_index);

            glVertexAttribPointer(weights_attribute_index, 4, GL_FLOAT, GL_FALSE, sizeof(render::Vertex), (void*)offsetof(render::Vertex, weights));
            glEnableVertexAttribArray(weights_attribute_index);

            glBindVertexArray(0);
        }

        GLuint load_texture2d(const tinygltf::Image& image, const tinygltf::Sampler& sampler) {
            GLuint texture_ {0};

            glGenTextures(1, &texture_);
            glBindTexture(GL_TEXTURE_2D, texture_);

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width, image.height,
                    0, GL_RGBA, image.pixel_type, image.image.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, sampler.wrapS);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, sampler.wrapT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, sampler.minFilter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, sampler.magFilter);
            glGenerateMipmap(GL_TEXTURE_2D);

            glBindTexture(GL_TEXTURE_2D, 0);
            return texture_;
        }

        GLuint load_texture2d(const std::string& texture, bool from_file) {
            GLuint texture_ {0};

            glGenTextures(1, &texture_);
            glBindTexture(GL_TEXTURE_2D, texture_);

            int width, height, component;
            unsigned char* data;
            if(from_file)
                data = stbi_load(texture.c_str(), &width, &height, &component, 0);
            else
                data = stbi_load_from_memory(reinterpret_cast<const unsigned char*>(texture.c_str()), texture.size(), &width, &height, &component, 0);
            
            glTexImage2D(GL_TEXTURE_2D, 0,
                    (component == 3 ? GL_RGB : GL_RGBA), width, height, 0,
                    (component == 3 ? GL_RGB : GL_RGBA), GL_UNSIGNED_BYTE, data);

            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
            glBindTexture(GL_TEXTURE_2D, 0);

            return texture_;
        }

        static GLuint compile_shader(const GLenum type, const std::string& code) {
            GLuint shader {glCreateShader(type)};
            const char* source = code.c_str();
            
            glShaderSource(shader, 1, &source, nullptr);
            glCompileShader(shader);

            int result {0};
            glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
            if(!result) {
                char log[512];
                glGetShaderInfoLog(shader, 512, NULL, log);
                auto info = std::format("failed to compile {} shader: {}", (shader == GL_VERTEX_SHADER ? "vertex" : "fragment"), log);
                glDeleteShader(shader);
                throw std::runtime_error(info);
            }

            return shader;
        }

        static void link_shaders(GLuint vertex, GLuint fragment, GLuint& program) {
            glAttachShader(program, vertex);
            glAttachShader(program, fragment);
            glLinkProgram(program);

            int result {0};
            glGetProgramiv(program, GL_LINK_STATUS, &result);
            if(!result) {
                char log[512];
                glGetProgramInfoLog(program, 512, nullptr, log);
                auto info = std::format("Failed to link shaders: {}", log);
                throw std::runtime_error(info);
            }

            glDeleteShader(vertex);
            glDeleteShader(fragment);
        }

        GLuint load_shader(const std::string& vertex, const std::string& fragment, bool from_file) {
            GLuint program = glCreateProgram();
            std::string vertex_code, fragment_code;

            if(from_file) {
                vertex_code = util::read_file(vertex);
                fragment_code = util::read_file(fragment);
            } else {
                vertex_code = vertex;
                fragment_code = fragment;
            }

            auto vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_code);
            auto fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_code);
            link_shaders(vertex_shader, fragment_shader, program);

            return program;
        }

        void load(SDL_Window* window, SDL_GLContext& context, uint32_t width, uint32_t height) {
            SDL_GL_LoadLibrary(NULL);
            SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        
            context = SDL_GL_CreateContext(window);
            if(!context) {
                auto exception = std::format("could not create SDL_GLContext: {}", SDL_GetError());
                throw std::runtime_error(exception);
            }
            SDL_GL_MakeCurrent(window, context);
        
            if(!gladLoadGLLoader(SDL_GL_GetProcAddress))
                throw std::runtime_error("could not load glad!");
        
            glViewport(0, 0, width, height);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);

#ifdef GLP_DEBUG
           int flags;
           glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
           if(flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
               glEnable(GL_DEBUG_OUTPUT);
               glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
               glDebugMessageCallback(opengl::debug, NULL);
               glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
           }
#endif
        }

#ifdef GLP_DEBUG
        void APIENTRY debug(GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length, const char* message, const void* user_param) {
            if(id == 131169 || id == 131185 || id == 131218 || id == 131204)
                return;
        
            char severity_str[7];
            char source_str[16];
            char type_str[21];
        
            switch(severity) {
                case GL_DEBUG_SEVERITY_HIGH:
                    strcpy(severity_str, "HIGH");
                    break;
                case GL_DEBUG_SEVERITY_MEDIUM:
                    strcpy(severity_str, "MID");
                    break;
                case GL_DEBUG_SEVERITY_LOW:
                    strcpy(severity_str, "LOW");
                    break;
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                    strcpy(severity_str, "NOTIFY");
                    break;
            }
        
            switch(source) {
                case GL_DEBUG_SOURCE_API:
                    strcpy(source_str, "API");
                    break;
                case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
                    strcpy(source_str, "Window System");
                    break;
                case GL_DEBUG_SOURCE_SHADER_COMPILER:
                    strcpy(source_str, "Shader Compiler");
                    break;
                case GL_DEBUG_SOURCE_THIRD_PARTY:
                    strcpy(source_str, "Third Party");
                    break;
                case GL_DEBUG_SOURCE_APPLICATION:
                    strcpy(source_str, "Application");
                    break;
                case GL_DEBUG_SOURCE_OTHER:
                    strcpy(source_str, "Other");
                    break;
            }
        
            switch(type) {
                case GL_DEBUG_TYPE_ERROR:
                    strcpy(type_str, "Error");
                    break;
                case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
                    strcpy(type_str, "Deprecated Behaviour");
                    break;
                case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
                    strcpy(type_str, "Undefined Behaviour");
                    break;
                case GL_DEBUG_TYPE_PORTABILITY:
                    strcpy(type_str, "Portability");
                    break;
                case GL_DEBUG_TYPE_PERFORMANCE:
                    strcpy(type_str, "Performance");
                    break;
                case GL_DEBUG_TYPE_MARKER:
                    strcpy(type_str, "Marker");
                    break;
                case GL_DEBUG_TYPE_PUSH_GROUP:
                    strcpy(type_str, "Push Group");
                    break;
                case GL_DEBUG_TYPE_POP_GROUP:
                    strcpy(type_str, "Pop Group");
                    break;
                case GL_DEBUG_TYPE_OTHER:
                    strcpy(type_str, "Other");
                    break;
            }
        
            fprintf(stderr, "[%s] OpenGL %s %s:\n\t%d %s\n",
                    severity_str,
                    source_str,
                    type_str,
                    id,
                    message);
        }
#endif
    }
}
