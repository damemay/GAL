#include <material.hh>
#include <sstream>
#include <gl.hh>

namespace glp {
    namespace render {
        constexpr auto static_vert = "#version 410 core\n"
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
        "void main() {\n"
        "    uv0 = texcoord0;\n"
        "    wpos = vec3(model * vec4(position, 1.0));\n"
        "    norm = transpose(inverse(mat3(model)))*normal;\n"
        "    gl_Position = vp * vec4(wpos, 1.0);\n"
        "}\n";

        constexpr auto skinned_vert = "#version 410 core\n"
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

        void Material::generate_shader() {
            std::stringstream sshader;
            sshader << "#version 410 core\n"
                "out vec4 out_color;\n"
                "struct Material {\n";
            if(albedo_id >= 0) sshader << "sampler2D albedo_tex;\n";
            else sshader << "vec4 albedo;\n";
            if(metallic_id >= 0) sshader << "sampler2D metallic_tex;\n";
            else sshader << "float metallic;\n";
            if(roughness_id >= 0) sshader << "sampler2D roughness_tex;\n";
            else sshader << "float roughness;\n";
            if(occlusion_id >= 0) sshader << "sampler2D occlusion_tex;\n";
            if(normal_id >= 0) sshader << "sampler2D normal_tex;\n";
            sshader << "};\n";
            sshader << "struct Light {\n"
                "vec3 position;\n"
                "vec3 direction;\n"
                "vec3 color;\n"
                "};\n"
                "struct Fog {\n"
                "vec3 color;\n"
                "float near;\n"
                "float far;\n"
                "};\n";
            sshader << "in vec2 uv0;\n"
                "in vec3 wpos;\n"
                "in vec3 norm;\n"
                "uniform vec3 camera_position;\n"
                "uniform Material material;\n"
                "uniform Light light;\n"
                "uniform Fog fog;\n";
            sshader << "float lin_depth(float depth) {\n"
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
                "}\n";
            sshader << "void main() {\n";
            if(albedo_id >= 0) sshader << "if(texture(material.albedo_tex, uv0).a < 0.1) discard;\n";
            sshader << "vec3 albedo;\n"
                "float metallic;\n"
                "float roughness;\n"
                "float ao;\n";
            if(albedo_id >= 0) sshader << "albedo = pow(texture(material.albedo_tex, uv0).rgb, vec3(2.2));\n";
            else sshader << "albedo = material.albedo.rgb;\n";
            if(metallic_id >= 0) sshader << "metallic = texture(material.metallic_tex, uv0).r;\n";
            else sshader << "metallic = material.metallic;\n";
            if(roughness_id >= 0) sshader << "roughness = texture(material.roughness_tex, uv0).r;\n";
            else sshader << "roughness = material.roughness;\n";
            if(occlusion_id >= 0) sshader << "ao = texture(material.occlusion_tex, uv0).r;\n";
            else sshader << "ao = 1.0f;\n";
            sshader << "vec3 normal;\n";
            if(normal_id >= 0) sshader << "vec3 tangent = texture(material.normal_tex, uv0).rgb * 2.0 - 1.0;\n"
                "vec3 q1 = dFdx(wpos);\n"
                "vec3 q2 = dFdy(wpos);\n"
                "vec2 st1 = dFdx(uv0);\n"
                "vec2 st2 = dFdy(uv0);\n"
                "vec3 n = normalize(norm);\n"
                "vec3 t = normalize(q1*st2.t-q2*st1.t);\n"
                "vec3 b = -normalize(cross(n,t));\n"
                "mat3 tbn = mat3(t,b,n);\n"
                "normal = normalize(tbn*tangent);\n";
            else sshader << "normal = normalize(norm);\n";
            sshader << "vec3 v = normalize(camera_position - wpos);\n"
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
                "vec3 ambient = vec3(0.03)*albedo*ao;\n"
                "vec3 color = ambient+lo;\n"
                "color = color/(color+vec3(1.0));\n"
                "color = pow(color, vec3(1.0/2.2));\n"
                "float fog_factor = lin_depth(gl_FragCoord.z)/fog.far;\n"
                "fog_factor = clamp(fog_factor, 0.0, 1.0);\n"
                "out_color = mix(vec4(color,1.0), vec4(fog.color,1.0), fog_factor);\n"
                "}\n";
            auto str_shader = sshader.str();
            shader = opengl::load_shader(static_vert, str_shader, false);

            use();
            uniforms.insert({"vp", glGetUniformLocation(shader, "vp")});
            uniforms.insert({"camera_position", glGetUniformLocation(shader, "camera_position")});
            uniforms.insert({"model", glGetUniformLocation(shader, "model")});
            if(albedo_id >= 0) uniforms.insert({"material.albedo_tex", glGetUniformLocation(shader, "material.albedo_tex")});
            else uniforms.insert({"material.albedo", glGetUniformLocation(shader, "material.albedo")});
            if(metallic_id >= 0) uniforms.insert({"material.metallic_tex", glGetUniformLocation(shader, "material.metallic_tex")});
            else uniforms.insert({"material.metallic", glGetUniformLocation(shader, "material.metallic")});
            if(roughness_id >= 0) uniforms.insert({"material.roughness_tex", glGetUniformLocation(shader, "material.roughness_tex")});
            else uniforms.insert({"material.roughness", glGetUniformLocation(shader, "material.roughness")});
            if(occlusion_id >= 0) uniforms.insert({"material.occlusion_tex", glGetUniformLocation(shader, "material.occlusion_tex")});
            if(normal_id >= 0) uniforms.insert({"material.normal_tex", glGetUniformLocation(shader, "material.normal_tex")});
            uniforms.insert({"light.position", glGetUniformLocation(shader, "light.position")});
            uniforms.insert({"light.direction", glGetUniformLocation(shader, "light.direction")});
            uniforms.insert({"light.color", glGetUniformLocation(shader, "light.color")});
            uniforms.insert({"fog.color", glGetUniformLocation(shader, "fog.color")});
            uniforms.insert({"fog.near", glGetUniformLocation(shader, "fog.near")});
            uniforms.insert({"fog.far", glGetUniformLocation(shader, "fog.far")});

            // if(albedo_id >= 0) set(uniforms.at("material.albedo_tex"), albedo_id);
            if(albedo_id == -1) set(uniforms.at("material.albedo"), albedo);
            // if(metallic_id >= 0) set(uniforms.at("material.metallic_tex"), metallic_id);
            if(metallic_id == -1) set(uniforms.at("material.metallic"), metallic);
            // if(roughness_id >= 0) set(uniforms.at("material.roughness_tex"), roughness_id);
            if(roughness_id == -1) set(uniforms.at("material.roughness"), roughness);
            // if(occlusion_id >= 0) set(uniforms.at("material.occlusion_tex"), occlusion_id);
            // if(normal_id >= 0) set(uniforms.at("material.normal_tex"), normal_id);
        }
    }
}
