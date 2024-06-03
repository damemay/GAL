#include <material.hh>
#include <gl.hh>

namespace gal {
    namespace render {
        void Material::setup_uniforms() {
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

            if(albedo_id == -1) set(uniforms.at("material.albedo"), albedo);
            if(metallic_id == -1) set(uniforms.at("material.metallic"), metallic);
            if(roughness_id == -1) set(uniforms.at("material.roughness"), roughness);
        }
    }
}
