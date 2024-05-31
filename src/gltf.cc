#include <cassert>
#include <stdexcept>
#include <format>
#include <gltf.hh>

namespace glp {
    namespace gltf {

        tinygltf::Model load_model(const std::string& path) {
            tinygltf::Model model;
            tinygltf::TinyGLTF loader;
            std::string err;
            std::string warn;
            bool ret = false;
        
            if(path.find(".glb") != std::string::npos) {
                ret = loader.LoadBinaryFromFile(&model, &err, &warn, path.c_str());
            } else if(path.find(".gltf") != std::string::npos) {
                ret = loader.LoadASCIIFromFile(&model, &err, &warn, path.c_str());
            } else {
                throw std::runtime_error("Provided non-GLTF filepath");
            }
        
        #ifdef CG_DEBUG
            if(!warn.empty()) printf("GLTF Warning: %s\n", warn.c_str());
            if(!err.empty()) printf("GLTF Error: %s\n", err.c_str());
        #endif
            if(!ret) {
                std::string info = std::format("Failed to load GLTF file: {}", path);
                throw std::runtime_error(info);
            }
            return model;
        }
        
        std::vector<render::Primitive> setup_primitives(tinygltf::Model& model) {
            std::vector<render::Primitive> meshes;
            for(const auto& mesh: model.meshes) {
                for(const auto& prim: mesh.primitives) {
                    std::vector<render::Vertex> vertices;
                    std::vector<unsigned int> indices;
        
                    if(const auto it = prim.attributes.find("POSITION"); it != std::end(prim.attributes)) {
                        const auto& accessor = model.accessors[it->second];
                        const auto& buffer_view = model.bufferViews[accessor.bufferView];
                        const auto& buffer = model.buffers[buffer_view.buffer];
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
                        const auto& accessor = model.accessors[it->second];
                        const auto& buffer_view = model.bufferViews[accessor.bufferView];
                        const auto& buffer = model.buffers[buffer_view.buffer];
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
                        const auto& accessor = model.accessors[it->second];
                        const auto& buffer_view = model.bufferViews[accessor.bufferView];
                        const auto& buffer = model.buffers[buffer_view.buffer];
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
                        const auto& accessor = model.accessors[prim.indices];
                        const auto& buffer_view = model.bufferViews[accessor.bufferView];
                        const auto& buffer = model.buffers[buffer_view.buffer];
                        assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
                        const unsigned short* index = reinterpret_cast<const unsigned short*>(&buffer.data[buffer_view.byteOffset + accessor.byteOffset]);
                        indices.resize(accessor.count);
                        for(size_t i=0; i<accessor.count; i++) {
                            indices[i] = index[i];
                        }
                    }
                    meshes.push_back(render::Primitive{vertices, indices});
                }
            }
        
            return meshes;
        }
    }
}

