#include "meshes.hh"
#include "render.hh"

Pose& Pose::operator=(const Pose& p) {
    if(&p == this) return *this;
    if(parents.size() != p.parents.size()) parents.resize(p.parents.size());
    if(joints.size() != p.joints.size()) joints.resize(p.joints.size());
    if(parents.size()) memcpy(&parents[0], &p.parents[0], sizeof(int)*parents.size());
    if(joints.size()) memcpy(&joints[0], &p.joints[0], sizeof(int)*joints.size());
    return *this;
}

transform::_t Pose::get_glob_transform(unsigned int idx) {
    transform::_t res = joints[idx];
    for(int p = parents[idx]; p>=0; p=parents[p])
        res = transform::combine(joints[p], res);
    return res;
}

void Pose::get_mat_palette(std::vector<glm::mat4>& out) {
    unsigned int size = this->size();
    if(out.size() != size) out.resize(size);
    for(unsigned int i=0; i<size; ++i) {
        transform::_t t = get_glob_transform(i);
        out[i] = transform::to_mat4(t);
    }
}

bool Pose::operator==(const Pose& p) {
    if(joints.size() != p.joints.size()) return false;
    if(parents.size() != p.parents.size()) return false;
    unsigned int size = joints.size();
    for(unsigned int i=0; i<size; ++i) {
        transform::_t t_local = joints[i];
        transform::_t p_local = p.joints[i];
        int t_parent = parents[i];
        int p_parent = p.parents[i];
        if(t_parent != p_parent) return false;
        if(t_local.position != p_local.position) return false;
        if(t_local.rotation != p_local.rotation) return false;
        if(t_local.scale != p_local.scale) return false;
    }
    return true;
}

void Skeleton::set(const Pose& rest, const Pose& bind, const std::vector<std::string>& names) {
    rest_pose = rest;
    bind_pose = bind;
    joint_names = names;
    update_inverse_bind_pose();
}

void Skeleton::update_inverse_bind_pose() {
    unsigned int size = bind_pose.size();
    inv_bind_pose.resize(size);
    for(unsigned int i=0; i<size; ++i) {
        transform::_t world = bind_pose.get_glob_transform(i);
        inv_bind_pose[i] = glm::inverse(transform::to_mat4(world));
    }
}

Mesh::Mesh() {
    pos_attrib = new Attribute<glm::vec3>();
    norm_attrib = new Attribute<glm::vec3>();
    uv_attrib = new Attribute<glm::vec2>();
    weight_attrib = new Attribute<glm::vec4>();
    influence_attrib = new Attribute<glm::ivec4>();
    index_buffer = new IndexBuffer;
}

Mesh::Mesh(const Mesh& m) {
    pos_attrib = new Attribute<glm::vec3>();
    norm_attrib = new Attribute<glm::vec3>();
    uv_attrib = new Attribute<glm::vec2>();
    weight_attrib = new Attribute<glm::vec4>();
    influence_attrib = new Attribute<glm::ivec4>();
    index_buffer = new IndexBuffer;
    *this = m;
}

Mesh& Mesh::operator=(const Mesh& m) {
    if(this == &m) return *this;
    position = m.position;
    normal = m.normal;
    tex_coord = m.tex_coord;
    weights = m.weights;
    influences = m.influences;
    indices = m.indices;
    update_gl_buffers();
    return *this;
}

Mesh::~Mesh() {
    delete pos_attrib;
    delete norm_attrib;
    delete uv_attrib;
    delete weight_attrib;
    delete influence_attrib;
    delete index_buffer;
}

void Mesh::update_gl_buffers() {
    if(position.size() > 0) pos_attrib->set(position);
    if(normal.size() > 0) norm_attrib->set(normal);
    if(tex_coord.size() > 0) uv_attrib->set(tex_coord);
    if(weights.size() > 0) weight_attrib->set(weights);
    if(influences.size() > 0) influence_attrib->set(influences);
    if(indices.size() > 0) index_buffer->set(indices);
}

void Mesh::bind(int pos, int norm, int tex, int weight, int inf) {
    if(pos >= 0) pos_attrib->bind_to(pos);
    if(norm >= 0) norm_attrib->bind_to(norm);
    if(tex >= 0) uv_attrib->bind_to(tex);
    if(weight >= 0) weight_attrib->bind_to(weight);
    if(inf >= 0) influence_attrib->bind_to(inf);
}

void Mesh::unbind(int pos, int norm, int tex, int weight, int inf) {
    if(pos >= 0) pos_attrib->unbind_from(pos);
    if(norm >= 0) norm_attrib->unbind_from(norm);
    if(tex >= 0) uv_attrib->unbind_from(tex);
    if(weight >= 0) weight_attrib->unbind_from(weight);
    if(inf >= 0) influence_attrib->unbind_from(inf);
}

void Mesh::render() {
    if(indices.size() > 0) render::render(*index_buffer, render::mode::triangles);
    else render::render(position.size(), render::mode::triangles);
}

void Mesh::render_instanced(unsigned int count) {
    if(indices.size() > 0) render::instanced(*index_buffer, render::mode::triangles, count);
    else render::instanced(position.size(), render::mode::triangles, count);
}
