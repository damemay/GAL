#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <material.hh>

namespace gal {
    namespace render {
        struct Shader_Generator_Booleans {
            bool albedo_texture;
            bool metallic_texture;
            bool roughness_texture;
            bool occlusion_texture;
            bool normal_texture;

            bool operator==(const Shader_Generator_Booleans& o) const { return (albedo_texture == o.albedo_texture && metallic_texture == o.metallic_texture && roughness_texture == o.roughness_texture && occlusion_texture == o.occlusion_texture && normal_texture == o.normal_texture); }
        };
    }
}

template <>
struct std::hash<gal::render::Shader_Generator_Booleans> {
    std::size_t operator()(const gal::render::Shader_Generator_Booleans& k) const {
        return ((std::hash<unsigned int>()(k.albedo_texture) ^ (std::hash<unsigned int>()(k.metallic_texture) << 1)) >> 1) ^ (std::hash<unsigned int>()(k.roughness_texture) << 1) ^ (std::hash<unsigned int>()(k.occlusion_texture) << 1) ^ (std::hash<unsigned int>()(k.normal_texture) << 1);
      }
};

namespace gal {
    namespace render {
        enum class Vertex_Shader_Type {
            Static,
            Skinned,
        };

        struct Shader_Generator {
            Vertex_Shader_Type vertex_shader_type;

            uint8_t shader_count;

            Shader_Generator(std::vector<Material*>& materials);

            private:
                uint8_t material_count;
                uint8_t no_texture_material_count;
                uint8_t albedo_texture_material_count;
                uint8_t metallic_texture_material_count;
                uint8_t roughness_texture_material_count;
                uint8_t occlusion_texture_material_count;
                uint8_t normal_texture_material_count;
                
                std::unordered_map<Shader_Generator_Booleans, GLuint> shaders;

                std::string generate_fragment_shader(const Shader_Generator_Booleans& booleans);
        };

        namespace shader_content {
            constexpr auto vertex_static = "#version 410 core\n"
            "layout (location = 0) in vec3 position;\n"
            "layout (location = 1) in vec3 normal;\n"
            "layout (location = 2) in vec2 texcoord0;\n"
            "layout (location = 3) in vec4 joints;\n"
            "layout (location = 4) in vec4 weights;\n"
            "out vec2 uv0;\n"
            "out vec3 wpos;\n"
            "out vec3 norm;\n"
            "out vec4 wpos_light_space;\n"
            "uniform mat4 vp;\n"
            "uniform mat4 model;\n"
            "uniform mat4 light_space_mat;\n"
            "void main() {\n"
            "    uv0 = texcoord0;\n"
            "    wpos = vec3(model * vec4(position, 1.0));\n"
            "    norm = transpose(inverse(mat3(model)))*normal;\n"
            "    wpos_light_space = light_space_mat * vec4(wpos, 1.0);\n"
            "    gl_Position = vp * vec4(wpos, 1.0);\n"
            "}\n";

            constexpr auto vertex_skinned = "#version 410 core\n"
            "layout (location = 0) in vec3 position;\n"
            "layout (location = 1) in vec3 normal;\n"
            "layout (location = 2) in vec2 texcoord0;\n"
            "layout (location = 3) in vec4 joints;\n"
            "layout (location = 4) in vec4 weights;\n"
            "out vec2 uv0;\n"
            "out vec3 wpos;\n"
            "out vec3 norm;\n"
            "uniform mat4 vp;\n"
            "uniform mat4 model;\n"
            "uniform mat4 pose[100];\n"
            "void main() {\n"
            "    mat4 skin = weights.x * pose[int(joints.x)] +\n"
            "                weights.y * pose[int(joints.y)] +\n"
            "                weights.z * pose[int(joints.z)] +\n"
            "                weights.w * pose[int(joints.w)];\n"
            "    uv0 = texcoord0;\n"
            "    wpos = vec3(model * vec4(position, 1.0));\n"
            "    norm = transpose(inverse(mat3(model)))*normal;\n"
            "    gl_Position = vp * skin * vec4(wpos, 1.0);\n"
            "}\n";

            constexpr auto fragment_0 =  "#version 410 core\n"
                "out vec4 out_color;\n"
                "struct Material {\n";
            constexpr auto fragment_1_albedo_tex = "sampler2D albedo_tex;\n";
            constexpr auto fragment_1_albedo = "vec4 albedo;\n";
            constexpr auto fragment_1_mettalic_tex = "sampler2D metallic_tex;\n";
            constexpr auto fragment_1_mettalic = "float metallic;\n";
            constexpr auto fragment_1_roughness_tex = "sampler2D roughness_tex;\n";
            constexpr auto fragment_1_roughness = "float roughness;\n";
            constexpr auto fragment_1_occlusion_tex = "sampler2D occlusion_tex;\n";
            constexpr auto fragment_1_normal_tex = "sampler2D normal_tex;\n";
            constexpr auto fragment_2 = "};\n"
                "struct Light {\n"
                "vec3 position;\n"
                "vec3 direction;\n"
                "vec3 color;\n"
                "};\n"
                "struct Fog {\n"
                "vec3 color;\n"
                "float near;\n"
                "float far;\n"
                "};\n"
                "in vec2 uv0;\n"
                "in vec3 wpos;\n"
                "in vec3 norm;\n"
                "in vec4 wpos_light_space;\n"
                "uniform vec3 camera_position;\n"
                "uniform Material material;\n"
                "uniform Light light;\n"
                "uniform Fog fog;\n"
                "uniform sampler2D shadow_map;\n"
                "float shadow_calc(vec4 wpos_lspace, float bias) {\n"
                "vec3 proj_coords = wpos_lspace.xyz / wpos_lspace.w;\n"
                //"if(proj_coords.z > 1.0) return 0.0;\n"
                "proj_coords = proj_coords * 0.5 + 0.5;\n"
                "float closest = texture(shadow_map, proj_coords.xy).r;\n"
                "float current = proj_coords.z;\n"
                "return current - bias > closest ? 1.0 : 0.0;\n"
                "}\n"
                "float lin_depth(float depth) {\n"
                "float z = depth*2.0-1.0;\n"
                "return (2.0*fog.near*fog.far)/(fog.far+fog.near-z*(fog.far-fog.near));\n"
                "}\n"
                "const float pi = 3.14159265359;\n"
                "vec3 fres(float cos_t, vec3 f0) {\n"
                "return f0+(1.0-f0)*pow(clamp(1.0-cos_t,0.0,1.0),5.0);\n"
                "}\n"
                "float dist_ggx(vec3 n, vec3 h, float roughness) {\n"
                "float a = roughness*roughness;\n"
                "float a2 = a*a;\n"
                "float ndoth = max(dot(n,h),0.0);\n"
                "float ndoth2 = ndoth*ndoth;\n"
                "float num=a2;\n"
                "float denom=(ndoth2*(a2-1.0)+1.0);\n"
                "denom=pi*denom*denom;\n"
                "return num/denom;\n"
                "}\n"
                "float geom_sch_ggx(float ndotv, float roughness) {\n"
                "float r = (roughness+1.0);\n"
                "float k = (r*r)/8.0;\n"
                "float num = ndotv;\n"
                "float denom = ndotv*(1.0-k)+k;\n"
                "return num/denom;\n"
                "}\n"
                "float geom_smith(vec3 n, vec3 v, vec3 l, float roughness) {\n"
                "float ndotv = max(dot(n,v),0.0);\n"
                "float ndotl = max(dot(n,l),0.0);\n"
                "float ggx2 = geom_sch_ggx(ndotv, roughness);\n"
                "float ggx1 = geom_sch_ggx(ndotl, roughness);\n"
                "return ggx1*ggx2;\n"
                "}\n"
                "void main() {\n"
                "vec3 albedo;\n"
                "float metallic;\n"
                "float roughness;\n"
                "vec3 normal;\n"
                "float ao;\n";
            constexpr auto fragment_3_albedo_tex = "if(texture(material.albedo_tex, uv0).a < 0.1) discard;\n"
                "albedo = pow(texture(material.albedo_tex, uv0).rgb, vec3(2.2));\n";
            constexpr auto fragment_3_albedo = "albedo = material.albedo.rgb;\n";
            constexpr auto fragment_3_metallic_tex = "metallic = texture(material.metallic_tex, uv0).r;\n";
            constexpr auto fragment_3_metallic = "metallic = material.metallic;\n";
            constexpr auto fragment_3_roughness_tex = "roughness = texture(material.roughness_tex, uv0).r;\n";
            constexpr auto fragment_3_roughness = "roughness = material.roughness;\n";
            constexpr auto fragment_3_occlusion_tex = "ao = texture(material.occlusion_tex, uv0).r;\n";
            constexpr auto fragment_3_occlusion = "ao = 1.0f;\n";
            constexpr auto fragment_3_normal_tex = "vec3 tangent = texture(material.normal_tex, uv0).rgb * 2.0 - 1.0;\n"
                "vec3 q1 = dFdx(wpos);\n"
                "vec3 q2 = dFdy(wpos);\n"
                "vec2 st1 = dFdx(uv0);\n"
                "vec2 st2 = dFdy(uv0);\n"
                "vec3 n = normalize(norm);\n"
                "vec3 t = normalize(q1*st2.t-q2*st1.t);\n"
                "vec3 b = -normalize(cross(n,t));\n"
                "mat3 tbn = mat3(t,b,n);\n"
                "normal = normalize(tbn*tangent);\n";
            constexpr auto fragment_3_normal = "normal = normalize(norm);\n";
            constexpr auto fragment_4 = "vec3 v = normalize(camera_position - wpos);\n"
                "vec3 f0 = vec3(0.04);\n"
                "f0 = mix(f0, albedo, metallic);\n"
                "vec3 lo = vec3(0.0);\n"
                "vec3 l;\n"
                "l = normalize(-light.direction);\n"
                "vec3 h = normalize(v+l);\n"
                "float dist;\n"
                "dist = length(-light.direction);\n"
                "float attent = 1.0/(dist*dist);\n"
                "vec3 rad = light.color*attent;\n"
                "float ndf = dist_ggx(normal,h,roughness);\n"
                "float g = geom_smith(normal,v,l,roughness);\n"
                "vec3 f = fres(max(dot(h,v),0.0),f0);\n"
                "vec3 ks = f;\n"
                "vec3 kd = vec3(1.0)-ks;\n"
                "kd *= 1.0-metallic;\n"
                "vec3 num = ndf*g*f;\n"
                "float denom = 4.0*max(dot(normal,v),0.0)*max(dot(normal,l),0.0)+0.0001;\n"
                "vec3 spec = num/denom;\n"
                "float ndotl = max(dot(normal,l),0.0);\n"
                "lo += (kd*albedo/pi+spec)*rad*ndotl;\n"
                "float bias = max(0.05 * (1.0 - dot(normal, light.direction)), 0.005);\n"
                "float shadow = shadow_calc(wpos_light_space, bias);\n"
                "vec3 ambient = vec3(0.03)*albedo*ao;\n"
                "vec3 color = ambient+((1.0-shadow)*lo);\n"
                "color = color/(color+vec3(1.0));\n"
                "color = pow(color, vec3(1.0/2.2));\n"
                "float fog_factor = lin_depth(gl_FragCoord.z)/fog.far;\n"
                "fog_factor = clamp(fog_factor, 0.0, 1.0);\n"
                "out_color = mix(vec4(color,1.0), vec4(fog.color,1.0), fog_factor);\n"
                "}\n";
        }
    }
}
