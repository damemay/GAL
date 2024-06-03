#include <util.hh>
#include <shader_generator.hh>
#include <sstream>
#include <gl.hh>

namespace gal {
    namespace render {
        Shader_Generator::Shader_Generator(std::vector<Material*>& materials) {
            for(auto& material: materials) {
                Shader_Generator_Booleans booleans = {
                    .albedo_texture = false,
                    .metallic_texture = false,
                    .roughness_texture = false,
                    .occlusion_texture = false,
                    .normal_texture = false,
                };
                if(material->albedo_id == -1
                        && material->metallic_id == -1
                        && material->roughness_id == -1
                        && material->occlusion_id == -1
                        && material->normal_id == -1) {
                    if(auto shader = shaders.find(booleans); shader == shaders.end()) {
                        auto fragment = generate_fragment_shader(booleans);
                        auto new_shader = opengl::load_shader(shader_content::vertex_static, fragment, false);
                        shaders.insert({booleans, new_shader});
                        material->shader = new_shader;
                    } else {
                        material->shader = shader->second;
                    }
                    continue;
                }
                if(material->albedo_id >= 0) booleans.albedo_texture = true;
                if(material->metallic_id >= 0) booleans.metallic_texture = true;
                if(material->roughness_id >= 0) booleans.roughness_texture = true;
                if(material->occlusion_id >= 0) booleans.occlusion_texture = true;
                if(material->normal_id >= 0) booleans.normal_texture = true;
                if(auto shader = shaders.find(booleans); shader == shaders.end()) {
                    auto fragment = generate_fragment_shader(booleans);
                    auto new_shader = opengl::load_shader(shader_content::vertex_static, fragment, false);
                    shaders.insert({booleans, new_shader});
                    material->shader = new_shader;
                } else {
                    material->shader = shader->second;
                }
            }
            util::print(std::format("generated {} shaders.", shaders.size()));
        }

        std::string Shader_Generator::generate_fragment_shader(const Shader_Generator_Booleans& booleans) {
            std::stringstream shaderstream;
            using namespace shader_content;
            shaderstream << fragment_0;
            if(booleans.albedo_texture) shaderstream << fragment_1_albedo_tex;
            else shaderstream << fragment_1_albedo;
            if(booleans.metallic_texture) shaderstream << fragment_1_mettalic_tex;
            else shaderstream << fragment_1_mettalic;
            if(booleans.roughness_texture) shaderstream << fragment_1_roughness_tex;
            else shaderstream << fragment_1_roughness;
            if(booleans.occlusion_texture) shaderstream << fragment_1_occlusion_tex;
            if(booleans.normal_texture) shaderstream << fragment_1_normal_tex;
            shaderstream << fragment_2;
            if(booleans.albedo_texture) shaderstream << fragment_3_albedo_tex;
            else shaderstream << fragment_3_albedo;
            if(booleans.metallic_texture) shaderstream << fragment_3_metallic_tex;
            else shaderstream << fragment_3_metallic;
            if(booleans.roughness_texture) shaderstream << fragment_3_roughness_tex;
            else shaderstream << fragment_3_roughness;
            if(booleans.occlusion_texture) shaderstream << fragment_3_occlusion_tex;
            else shaderstream << fragment_3_occlusion;
            if(booleans.normal_texture) shaderstream << fragment_3_normal_tex;
            else shaderstream << fragment_3_normal;
            shaderstream << fragment_4;
            return shaderstream.str();
        }
    }
}
