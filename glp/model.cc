#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <cassert>

#include "model.hh"
#include "utils.h"

static std::vector<GLuint> gen_buffer_objects(const tinygltf::Model& model) {
    std::vector<GLuint> bufs(model.buffers.size(), 0);

    glGenBuffers((GLsizei)model.buffers.size(), bufs.data());

    for(size_t i=0; i<model.buffers.size(); ++i) {
        glBindBuffer(GL_ARRAY_BUFFER, bufs[i]);
        glBufferData(GL_ARRAY_BUFFER, model.buffers[i].data.size(), model.buffers[i].data.data(), 0);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return bufs;
}

static bool find_and_point(const std::string& attr_n, const GLuint index,
        const tinygltf::Model& model, const tinygltf::Primitive& prim, const std::vector<GLuint>& bufs) {
    if(prim.attributes.find(attr_n) == prim.attributes.end()) return false;
    const auto& acc = model.accessors[prim.attributes.find(attr_n)->second];
    const auto& buf_v = model.bufferViews[acc.bufferView];
    const auto buf_idx = buf_v.buffer;
    glEnableVertexAttribArray(index);
    if(GL_ARRAY_BUFFER!=buf_v.target) return false;
    glBindBuffer(GL_ARRAY_BUFFER, bufs[buf_idx]);
    const auto byte_off = acc.byteOffset + buf_v.byteOffset;
    glVertexAttribPointer(index, acc.type, acc.componentType, GL_FALSE, (GLsizei)buf_v.byteStride, (const GLvoid*)byte_off);
    return true;
}

static std::vector<GLuint> gen_vaos(const tinygltf::Model& model, const std::vector<GLuint>& bufs, std::vector<vao_range>& mesh2vao) {
    std::vector<GLuint> vaos;

    mesh2vao.resize(model.meshes.size());

    const GLuint ATTRIB_POSITION_IDX = 0;
    const GLuint ATTRIB_NORMAL_IDX = 0;
    const GLuint ATTRIB_TEXCOORD0_IDX = 0;

    for(size_t i=0; i<model.meshes.size(); ++i) {
        const auto& mesh = model.meshes[i];
        auto& vao_r = mesh2vao[i];
        vao_r.begin = (GLsizei)vaos.size();
        vao_r.count = (GLsizei)mesh.primitives.size();

        vaos.resize(vaos.size() + mesh.primitives.size());
        glGenVertexArrays(vao_r.count, &vaos[vao_r.begin]);

        for(size_t pi=0; pi<mesh.primitives.size(); ++pi) {
            const auto vao = vaos[vao_r.begin+pi];
            const auto& prim = mesh.primitives[pi];

            glBindVertexArray(vao);
            if(!find_and_point("POSITION", ATTRIB_POSITION_IDX, model, prim, bufs))
                glp_log("error when looking up position attribute in gltf");
            if(!find_and_point("NORMAL", ATTRIB_NORMAL_IDX, model, prim, bufs))
                glp_log("error when looking up normal attribute in gltf");
            if(!find_and_point("TEXCOORD_0", ATTRIB_TEXCOORD0_IDX, model, prim, bufs))
                glp_log("error when looking up normal attribute in gltf");
            if(prim.indices >= 0) {
                const auto& acc = model.accessors[prim.indices];
                const auto& buf_v = model.bufferViews[acc.bufferView];
                const auto buf_idx = buf_v.buffer;
                assert(GL_ELEMENT_ARRAY_BUFFER == buf_v.target);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bufs[buf_idx]);
            }
        }
    }
    glBindVertexArray(0);

    glp_logv("gltf total vaos number: %zu", vaos.size());

    return vaos;
}

bool Model::load(std::string filename) {
    glp_logv("loading gltf %s", filename.c_str());

    tinygltf::TinyGLTF ctx;
    tinygltf::Model model;
    std::string err, warn;

    bool binary = false;
    size_t extpos = filename.rfind('.', filename.length());
    if(extpos != std::string::npos) binary = (filename.substr(extpos+1, filename.length() - extpos) == "glb");
    
    bool loaded = binary ? ctx.LoadBinaryFromFile(&model, &err, &warn, filename) : ctx.LoadASCIIFromFile(&model, &err, &warn, filename);
    if(!loaded) {
        glp_logv("could not load gltf: %s", err.c_str());
        return false;
    }

    buffer_objects = gen_buffer_objects(model);
    vaos = gen_vaos(model, buffer_objects, mesh_vaos);

    glp_logv("loaded %s", filename.c_str());
    return true;
}

Model::Model(std::string path) {
    if(!load(path)) throw "Could not create Model with GLTF file";
}

Model::~Model() {

}
