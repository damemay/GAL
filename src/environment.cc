#include <environment.hh>

namespace gal {
    namespace render {
        void Fog::set(const render::Material& material) {
            material.set(material.uniforms.at("fog.color"), color);
            material.set(material.uniforms.at("fog.near"), near);
            material.set(material.uniforms.at("fog.far"), far);
        }

        void Light::set(const render::Material& material) {
            material.set(material.uniforms.at("light.position"), position);
            material.set(material.uniforms.at("light.color"), color);
        }

        void Directional_Light::set(const render::Material& material) {
            Light::set(material);
            material.set(material.uniforms.at("light.direction"), direction);
        }

        Shadow_Map::Shadow_Map() {
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                    shadow_wh, shadow_wh, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glGenFramebuffers(1, &framebuffer);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                    texture, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glBindTexture(GL_TEXTURE_2D, 0);

            shader.compile(shadow_vertex_shader, shader_fragment_shader, false);
        }
    }
}
