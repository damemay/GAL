#include <cassert>

#include "model.hh"
#include "utils.hh"

constexpr GLuint POSITION_ATTRIBUTE_INDEX       = 0;
constexpr GLuint NORMAL_ATTRIBUTE_INDEX         = 1;
constexpr GLuint TEXCOORD0_ATTRIBUTE_INDEX      = 2;
constexpr GLuint JOINTS_ATTRIBUTE_INDEX         = 3;
constexpr GLuint WEIGHTS_ATTRIBUTE_INDEX        = 4;

bool Model::attribute_mesh(const std::string& name, const GLuint index, Mesh*& new_mesh, const tinygltf::Model& model, const tinygltf::Primitive& primitive) {
    if(primitive.attributes.find(name) != end(primitive.attributes)) {
        auto& accessor_index = primitive.attributes.find(name)->second;
        auto& accessor = model.accessors[accessor_index];
        auto& buffer_view = model.bufferViews[accessor.bufferView];
        auto& buffer_index = buffer_view.buffer;
        new_mesh->vao_index = buffer_index;
        new_mesh->count = accessor.count;
    
        glBindVertexArray(VAO[buffer_index]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO[buffer_index]);
        glVertexAttribPointer(index, accessor.type, accessor.componentType, GL_FALSE,
                buffer_view.byteStride, (const GLvoid*)(accessor.byteOffset + buffer_view.byteOffset));
        glEnableVertexAttribArray(index);
        glp_logv("attributed %ss", name.c_str());
        return true;
    }
    return false;
}

Mesh* Model::load_mesh(const tinygltf::Model& model, const tinygltf::Mesh& mesh) {
    auto new_mesh = new Mesh();
    for(size_t j=0; j<mesh.primitives.size(); j++) {
        auto& prim = mesh.primitives[j];
        if(prim.mode > -1) new_mesh->mode = prim.mode;
        auto& attr = prim.attributes;
    
        attribute_mesh("POSITION", POSITION_ATTRIBUTE_INDEX, new_mesh, model, prim);
        attribute_mesh("NORMAL", NORMAL_ATTRIBUTE_INDEX, new_mesh, model, prim);
        attribute_mesh("TEXCOORD_0", TEXCOORD0_ATTRIBUTE_INDEX, new_mesh, model, prim);
        attribute_mesh("JOINTS_0", JOINTS_ATTRIBUTE_INDEX, new_mesh, model, prim);
        attribute_mesh("WEIGHTS_0", WEIGHTS_ATTRIBUTE_INDEX, new_mesh, model, prim);

        int indices = prim.indices;
        if(indices > -1) {
            auto& accessor_index = indices;
            auto& accessor = model.accessors[accessor_index];
            auto& buffer_view = model.bufferViews[accessor.bufferView];
            auto& buffer_index = buffer_view.buffer;
            new_mesh->has_indices = true;
            new_mesh->indices.count = accessor.count;
            new_mesh->indices.offset = buffer_view.byteOffset + accessor.byteOffset;
            new_mesh->indices.type = accessor.componentType;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[buffer_index]);
        }
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return new_mesh;
}

void Model::load_node(Node* parent, const tinygltf::Node& node, const tinygltf::Model& model) {
    Node* new_node = new Node();
    new_node->parent = parent;

    glp_logv("parent: %p", parent);

    if(!node.translation.empty())
        new_node->translation = glm::make_vec3(node.translation.data());
    
    if(!node.rotation.empty())
        new_node->rotation = glm::make_quat(node.rotation.data());
    
    if(!node.scale.empty())
        new_node->scale = glm::make_vec3(node.scale.data());
    
    if(!node.matrix.empty())
        new_node->matrix = glm::make_mat4x4(node.matrix.data());

    if(node.mesh > -1)
        new_node->mesh = load_mesh(model, model.meshes[node.mesh]);

    if(!node.children.empty())
        for(const auto& child: node.children)
            load_node(new_node, model.nodes[child], model);

    glp_logv("children count: %lu", new_node->children.size());

    if(parent) parent->children.push_back(new_node);
    else nodes.push_back(new_node);
}

void Model::gen_buffers(const tinygltf::Model& model) {
    VBO.resize(model.buffers.size(), 0);
    glGenBuffers(VBO.size(), VBO.data());
    for(size_t i=0; i<VBO.size(); i++) {
        glBindBuffer(GL_ARRAY_BUFFER, VBO[i]);
        auto& buffer = model.buffers[i];
        glBufferData(GL_ARRAY_BUFFER, buffer.data.size(),
                buffer.data.data(), GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    VAO.resize(VBO.size(), 0);
    glGenVertexArrays(VBO.size(), VAO.data());
}

bool Model::load(const std::string& filename) {
    glp_logv("loading %s", filename.c_str());

    tinygltf::TinyGLTF ctx;
    tinygltf::Model model;
    std::string err, warn;

    bool binary = false;
    size_t extpos = filename.rfind('.', filename.length());
    if(extpos != std::string::npos) binary = (filename.substr(extpos+1, filename.length() - extpos) == "glb");
    
    bool loaded = binary ? ctx.LoadBinaryFromFile(&model, &err, &warn, filename) : ctx.LoadASCIIFromFile(&model, &err, &warn, filename);
    if(!loaded) {
        glp_logv("could not load: %s", err.c_str());
        return false;
    }

    gen_buffers(model);

    for(size_t i=0; i<model.scenes[0].nodes.size(); i++)
        load_node(nullptr, model.nodes[i], model);
    glp_logv("node count: %lu", nodes.size());

    for(const tinygltf::Texture& tex: model.textures) {
        auto image = model.images[tex.source];
        textures.emplace_back(image);
    }
    glp_logv("texture count: %lu", textures.size());

    glp_logv("loaded %s", filename.c_str());
    return true;
}

void Model::render_node(Node* node, glm::mat4 transform, const glm::mat4& mat, const std::string& uniform) {
    transform *= node->matrix;
    transform *= glm::translate(glm::mat4(1.0f), node->translation) * glm::mat4(node->rotation) * glm::scale(glm::mat4(1.0f), node->scale);

    if(node->mesh) {
        auto& mesh = node->mesh;
    
        shader->bind();
        shader->set(uniform, mat);
        shader->set("model", transform);
    
        for(size_t i=0; i<textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textures[i].get());
            char tex_name[128];
            sprintf(tex_name, "tex%lu", i);
            shader->set(tex_name, (int)i);
        }
        glBindVertexArray(VAO[mesh->vao_index]);
        if(!mesh->has_indices) glDrawArrays(mesh->mode, 0, mesh->count);
        else glDrawElements(mesh->mode, mesh->indices.count,
                mesh->indices.type, (const GLvoid*)mesh->indices.offset);
    
        shader->unbind();
    }

    if(!node->children.empty())
        for(auto& child: node->children)
            render_node(child, transform, mat, uniform);
}

void Model::render(const glm::mat4& mat, const std::string& uniform) {
    for(auto& node: nodes) render_node(node, glm::mat4(1.0f), mat, uniform);
}

Model::Model(const std::string& path, Shader& shader_) {
    if(!load(path)) throw "Could not create Model with GLTF file";
    shader = &shader_;
}

Model::~Model() {
    for(auto& node: nodes) delete node;
    glDeleteBuffers(VBO.size(), VBO.data());
    glDeleteVertexArrays(VAO.size(), VAO.data());
}
