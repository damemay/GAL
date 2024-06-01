#include <cassert>
#include <stdexcept>
#include <format>
#include <gl.hh>
#include <map>
#include <gltf.hh>

namespace gal {
    namespace gltf {
        Model::Model(const std::string& path) {
            load(path);
            setup_primitives();
        }

        void Model::load(const std::string& path) {
            tinygltf::TinyGLTF loader;
            std::string err;
            std::string warn;
            bool ret = false;
        
            if(path.find(".glb") != std::string::npos) {
                ret = loader.LoadBinaryFromFile(&tinygltf_model, &err, &warn, path.c_str());
            } else if(path.find(".gltf") != std::string::npos) {
                ret = loader.LoadASCIIFromFile(&tinygltf_model, &err, &warn, path.c_str());
            } else {
                throw std::runtime_error("Provided non-GLTF filepath");
            }
        
        #ifdef GAL_DEBUG
            if(!warn.empty()) printf("GLTF Warning: %s\n", warn.c_str());
            if(!err.empty()) printf("GLTF Error: %s\n", err.c_str());
        #endif
            if(!ret) {
                std::string info = std::format("Failed to load GLTF file: {}", path);
                throw std::runtime_error(info);
            }
        }
        
        void Model::setup_primitives() {
            tinygltf::Sampler default_sampler;
            default_sampler.wrapS = GL_REPEAT;
            default_sampler.wrapT = GL_REPEAT;
            default_sampler.minFilter = GL_LINEAR_MIPMAP_LINEAR;
            default_sampler.magFilter = GL_LINEAR;
            for(const auto& mesh: tinygltf_model.meshes) {
                for(const auto& prim: mesh.primitives) {
                    std::vector<render::Vertex> vertices;
                    std::vector<unsigned int> indices;
        
                    if(const auto it = prim.attributes.find("POSITION"); it != std::end(prim.attributes)) {
                        const auto& accessor = tinygltf_model.accessors[it->second];
                        const auto& buffer_view = tinygltf_model.bufferViews[accessor.bufferView];
                        const auto& buffer = tinygltf_model.buffers[buffer_view.buffer];
                        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                        const float* position = reinterpret_cast<const float*>(&buffer.data[buffer_view.byteOffset + accessor.byteOffset]);
                        vertices.resize(accessor.count);
                        for(size_t i=0; i<accessor.count; i++) {
                            vertices[i].position = glm::vec3{
                                position[i*3+0],
                                position[i*3+1],
                                position[i*3+2]
                            };
                        }
                    }
                    if(const auto it = prim.attributes.find("NORMAL"); it != std::end(prim.attributes)) {
                        const auto& accessor = tinygltf_model.accessors[it->second];
                        const auto& buffer_view = tinygltf_model.bufferViews[accessor.bufferView];
                        const auto& buffer = tinygltf_model.buffers[buffer_view.buffer];
                        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                        const float* normal = reinterpret_cast<const float*>(&buffer.data[buffer_view.byteOffset + accessor.byteOffset]);
                        assert(vertices.size() == accessor.count);
                        for(size_t i=0; i<accessor.count; i++) {
                            vertices[i].normal = glm::vec3{
                                normal[i*3+0],
                                normal[i*3+1],
                                normal[i*3+2]
                            };
                        }
                    }
                    if(const auto it = prim.attributes.find("TEXCOORD_0"); it != std::end(prim.attributes)) {
                        const auto& accessor = tinygltf_model.accessors[it->second];
                        const auto& buffer_view = tinygltf_model.bufferViews[accessor.bufferView];
                        const auto& buffer = tinygltf_model.buffers[buffer_view.buffer];
                        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
                        const float* uv = reinterpret_cast<const float*>(&buffer.data[buffer_view.byteOffset + accessor.byteOffset]);
                        assert(vertices.size() == accessor.count);
                        for(size_t i=0; i<accessor.count; i++) {
                            vertices[i].uv = glm::vec2{
                                uv[i*2+0],
                                uv[i*2+1],
                            };
                        }
                    }
                    if(prim.indices >= 0) {
                        const auto& accessor = tinygltf_model.accessors[prim.indices];
                        const auto& buffer_view = tinygltf_model.bufferViews[accessor.bufferView];
                        const auto& buffer = tinygltf_model.buffers[buffer_view.buffer];
                        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
                        const unsigned short* index = reinterpret_cast<const unsigned short*>(&buffer.data[buffer_view.byteOffset + accessor.byteOffset]);
                        indices.resize(accessor.count);
                        for(size_t i=0; i<accessor.count; i++) {
                            indices[i] = index[i];
                        }
                    }

                    render::Material material {};
                    if(prim.material >= 0) {
                        const auto& mat = tinygltf_model.materials[prim.material];
                        auto base_color = mat.pbrMetallicRoughness.baseColorFactor;
                        material.albedo = glm::vec4{base_color[0], base_color[1], base_color[2], base_color[3]};
                        material.metallic = mat.pbrMetallicRoughness.metallicFactor;
                        material.roughness = mat.pbrMetallicRoughness.roughnessFactor;

                        if(auto id = mat.pbrMetallicRoughness.baseColorTexture.index; id >= 0) {
                            material.albedo_id = id;
                            const auto& texture = tinygltf_model.textures[id];
                            const auto& sampler_id = texture.sampler;
                            const tinygltf::Sampler& sampler = sampler_id >= 0 ? tinygltf_model.samplers[sampler_id] : default_sampler;
                            const auto& image_id = texture.source;
                            const auto& image = tinygltf_model.images[image_id];
                            material.textures.insert({id, opengl::load_texture2d(image, sampler)});
                        }
                        if(auto id = mat.pbrMetallicRoughness.metallicRoughnessTexture.index; id >= 0) {
                            material.metallic_id = id;
                            const auto& texture = tinygltf_model.textures[id];
                            const auto& sampler_id = texture.sampler;
                            const tinygltf::Sampler& sampler = sampler_id >= 0 ? tinygltf_model.samplers[sampler_id] : default_sampler;
                            const auto& image_id = texture.source;
                            const auto& image = tinygltf_model.images[image_id];
                            material.textures.insert({id, opengl::load_texture2d(image, sampler)});
                        }
                        if(auto id = mat.normalTexture.index; id >= 0) {
                            material.normal_id = id;
                            const auto& texture = tinygltf_model.textures[id];
                            const auto& sampler_id = texture.sampler;
                            const tinygltf::Sampler& sampler = sampler_id >= 0 ? tinygltf_model.samplers[sampler_id] : default_sampler;
                            const auto& image_id = texture.source;
                            const auto& image = tinygltf_model.images[image_id];
                            material.textures.insert({id, opengl::load_texture2d(image, sampler)});
                        }
                        if(auto id = mat.occlusionTexture.index; id >= 0) {
                            material.occlusion_id = id;
                            const auto& texture = tinygltf_model.textures[id];
                            const auto& sampler_id = texture.sampler;
                            const tinygltf::Sampler& sampler = sampler_id >= 0 ? tinygltf_model.samplers[sampler_id] : default_sampler;
                            const auto& image_id = texture.source;
                            const auto& image = tinygltf_model.images[image_id];
                            material.textures.insert({id, opengl::load_texture2d(image, sampler)});
                        }
                    }

                    primitives.insert({render::Primitive{vertices, indices}, material});
                }
            }
        }
    }
}

