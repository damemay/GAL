#include <cassert>
#include <cstddef>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>
#include <limits>

#include <material.hh>
#include <model.hh>
#include <utils.hh>

namespace glp {

Mesh::Mesh(std::vector<Vertex> vert, std::vector<unsigned int> idx, Material* mat, Shader* shader)
    : vertices{vert}, indices{idx}, material{mat} {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(Vertex),
            vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int),
            indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(POSITION_ATTRIBUTE_INDEX,
            3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(POSITION_ATTRIBUTE_INDEX);

    glVertexAttribPointer(NORMAL_ATTRIBUTE_INDEX,
            3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(NORMAL_ATTRIBUTE_INDEX);

    glVertexAttribPointer(TEXCOORD0_ATTRIBUTE_INDEX,
            2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));
    glEnableVertexAttribArray(TEXCOORD0_ATTRIBUTE_INDEX);

    glVertexAttribPointer(JOINTS_ATTRIBUTE_INDEX,
            4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bone_index));
    glEnableVertexAttribArray(JOINTS_ATTRIBUTE_INDEX);

    glVertexAttribPointer(WEIGHTS_ATTRIBUTE_INDEX,
            4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));
    glEnableVertexAttribArray(WEIGHTS_ATTRIBUTE_INDEX);

    glBindVertexArray(0);
}

void Mesh::render(Shader* shader, ShadingType type) {
    uint8_t count = 0;
    if(material->diffuse_id>-1) {
        glActiveTexture(GL_TEXTURE0+count);
        shader->set("material.diffuse_tex_exists", 1);
        shader->set("material.diffuse_tex", count);
        glBindTexture(GL_TEXTURE_2D, material->textures[material->diffuse_id]->id);
        count++;
    } else {
        shader->set("material.diffuse_tex_exists", 0);
        if(type==ShadingType::PHONG) {
            shader->set("material.diffuse", material->diffuse);
            shader->set("material.ambient", material->ambient);
            shader->set("material.shininess", material->shininess);
        } else shader->set("material.albedo", material->albedo);
    }

    if(material->normal_id>-1) {
        glActiveTexture(GL_TEXTURE0+count);
        shader->set("material.normal_tex_exists", 1);
        shader->set("material.normal_tex", count);
        glBindTexture(GL_TEXTURE_2D, material->textures[material->normal_id]->id);
        count++;
    } else shader->set("material.normal_tex_exists", 0);

    if(type == ShadingType::PHONG) {
        if(material->specular_id>-1) {
            glActiveTexture(GL_TEXTURE0+count);
            shader->set("material.specular_tex_exists", 1);
            shader->set("material.specular_tex", count);
            glBindTexture(GL_TEXTURE_2D, material->textures[material->specular_id]->id);
            count++;
        } else {
            shader->set("material.specular_tex_exists", 0);
            shader->set("material.specular", material->specular);
        }
    } else {
        if(material->metallic_id>-1) {
            glActiveTexture(GL_TEXTURE0+count);
            shader->set("material.metallic_tex_exists", 1);
            shader->set("material.metallic_tex", count);
            glBindTexture(GL_TEXTURE_2D, material->textures[material->metallic_id]->id);
            count++;
        } else {
            shader->set("material.metallic_tex_exists", 0);
            shader->set("material.metallic", material->metallic);
        }

        if(material->roughness_id>-1) {
            glActiveTexture(GL_TEXTURE0+count);
            shader->set("material.roughness_tex_exists", 1);
            shader->set("material.roughness_tex", count);
            glBindTexture(GL_TEXTURE_2D, material->textures[material->roughness_id]->id);
            count++;
        } else {
            shader->set("material.roughness_tex_exists", 0);
            shader->set("material.roughness", material->roughness);
        }

        if(material->ao_id>-1) {
            glActiveTexture(GL_TEXTURE0+count);
            shader->set("material.ao_tex_exists", 1);
            shader->set("material.ao_tex", count);
            glBindTexture(GL_TEXTURE_2D, material->textures[material->ao_id]->id);
            count++;
        } else {
            shader->set("material.ao_tex_exists", 0);
        }
    }

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

Mesh::~Mesh() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

Model::Model(const std::string& path, Shader* shader_, ShadingType shading_t) 
    : shader{shader_}, shading{shading_t} {
    load(path);
}

void Model::load(const std::string& path) {
    auto file = util::read_file(path);
    directory = path.substr(0, path.find_last_of('/'));
    auto decompressed = util::decompress(file);
    std::stringstream s;
    s << decompressed;
    deserialize_data(s);
}

Texture* Model::texture_load(const std::string& path) {
    Texture* tex;
    bool skip = false;
    for(Texture* loaded: textures) {
        if(loaded->path == path) {
            tex = loaded;
            skip = true;
            break;
        }
    }
    if(!skip) {
        tex = new Texture{path};
        glp_logv("new texture: %s", tex->path.c_str());
        textures.push_back(tex);
    }
    return tex;
}

void Model::render() {
    for(auto& mesh: meshes) mesh->render(shader, shading);
}

glm::vec3 Model::calculate_bounding_box() {
    glm::vec3 min {std::numeric_limits<float>::max()};
    glm::vec3 max {-std::numeric_limits<float>::max()};
    for(const auto& mesh: meshes) {
        for(const auto& vert: mesh->vertices) {
            min.x = std::min(min.x, vert.position.x);
            min.y = std::min(min.y, vert.position.y);
            min.z = std::min(min.z, vert.position.z);
            max.x = std::max(max.x, vert.position.x);
            max.y = std::max(max.y, vert.position.y);
            max.z = std::max(max.z, vert.position.z);
        }
    }
    return glm::vec3((max.x-min.x)/2,
            (max.y-min.y)/2,
            (max.z-min.z)/2);
}

Model::~Model() {
    for(auto& mesh: meshes) delete mesh;
    for(auto& tex: textures) delete tex;
}

std::stringstream Model::serialize_data() {
    std::stringstream s;
    s << "meshes " << meshes.size() << ' ';
    for(const auto& mesh: meshes) {
        s << "mesh " << "verts " << mesh->vertices.size() << ' ';
        for(const auto& vert: mesh->vertices) {
            s << "vert ";
            s << "p " << vert.position.x << ' '
                << vert.position.y << ' '
                << vert.position.z << ' ';
            s << "n " << vert.normal.x << ' '
                << vert.normal.y << ' '
                << vert.normal.z << ' ';
            s << "u " << vert.uv.x << ' '
                << vert.uv.y << ' ' << ' ';
            s << "b " << vert.bone_index[0] << ' '
                << vert.bone_index[1] << ' '
                << vert.bone_index[2] << ' '
                << vert.bone_index[3] << ' ';
            s << "w " << vert.weights[0] << ' '
                << vert.weights[1] << ' '
                << vert.weights[2] << ' '
                << vert.weights[3] << ' ';
        }
        s << "idxs " << mesh->indices.size() << ' ';
        for(const auto& id: mesh->indices)
            s << id << ' ';
        s << "mat " << "texs " << mesh->material->textures.size() << ' ';
        for(const auto& tex: mesh->material->textures) {
            size_t f = tex->path.find_last_of("/");
            s << tex->path.substr(f+1) << ' ';
        }
        s << "amb " << mesh->material->ambient.x << ' '
            << mesh->material->ambient.y << ' '
            << mesh->material->ambient.z << ' ';
        s << "dif " << mesh->material->diffuse.x << ' '
            << mesh->material->diffuse.y << ' '
            << mesh->material->diffuse.z << ' ';
        s << "spc " << mesh->material->specular.x << ' '
            << mesh->material->specular.y << ' '
            << mesh->material->specular.z << ' ';
        s << "shn " << mesh->material->shininess << ' ';
        s << "alb " << mesh->material->albedo.x << ' '
            << mesh->material->albedo.y << ' '
            << mesh->material->albedo.z << ' ';
        s << "mtl " << mesh->material->metallic << ' ';
        s << "rgh " << mesh->material->roughness << ' ';
        s << "did " << mesh->material->diffuse_id << ' ';
        s << "sid " << mesh->material->specular_id << ' ';
        s << "mid " << mesh->material->metallic_id << ' ';
        s << "rid " << mesh->material->roughness_id << ' ';
        s << "aid " << mesh->material->ao_id << ' ';
        s << "nid " << mesh->material->normal_id << ' ';
       // s << "texs " << mesh->textures.size() << ' ';
       // for(const auto& tex: mesh->textures)
       //     s << tex->path << ' ';
    }

    s << "bones " << bones.size() << ' ';
    for(const auto& bone: bones) {
        s << "b " << bone.name << ' ';
        s << bone.offset[0].x << ' '
            << bone.offset[0].y << ' '
            << bone.offset[0].z << ' '
            << bone.offset[0].w << ' '
            << bone.offset[1].x << ' '
            << bone.offset[1].y << ' '
            << bone.offset[1].z << ' '
            << bone.offset[1].w << ' '
            << bone.offset[2].x << ' '
            << bone.offset[2].y << ' '
            << bone.offset[2].z << ' '
            << bone.offset[2].w << ' '
            << bone.offset[3].x << ' '
            << bone.offset[3].y << ' '
            << bone.offset[3].z << ' '
            << bone.offset[3].w << ' ';
    }

    return s;
}

void Model::deserialize_data(std::stringstream& s) {
    std::string name;
    size_t count;
    
    s >> name >> count; assert("meshes");
    if(count > 0) meshes.resize(count);
    
    for(size_t i=0; i<meshes.size(); i++) {
        s >> name; assert(name == "mesh");
        s >> name; assert(name == "verts");
        s >> count;
        std::vector<Vertex> verts;
        if(count > 0) verts.resize(count);
        for(size_t i=0; i<verts.size(); i++) {
            s >> name; assert(name == "vert");
            s >> name; assert(name == "p");
            s >> verts[i].position.x
                >> verts[i].position.y
                >> verts[i].position.z;
            s >> name; assert(name == "n");
            s >> verts[i].normal.x
                >> verts[i].normal.y
                >> verts[i].normal.z;
            s >> name; assert(name == "u");
            s >> verts[i].uv.x
                >> verts[i].uv.y;
            s >> name; assert(name == "b");
            s >> verts[i].bone_index[0]
                >> verts[i].bone_index[1]
                >> verts[i].bone_index[2]
                >> verts[i].bone_index[3];
            s >> name; assert(name == "w");
            s >> verts[i].weights[0]
                >> verts[i].weights[1]
                >> verts[i].weights[2]
                >> verts[i].weights[3];
        }
        s >> name; assert(name == "idxs");
        s >> count;
        std::vector<unsigned int> idxs;
        if(count > 0) idxs.resize(count);
        for(size_t i=0; i<idxs.size(); i++)
            s >> idxs[i];
        s >> name; assert(name == "mat");
        auto mat = new Material;
        s >> name; assert(name == "texs");
        s >> count;
        mat->textures.resize(count);
        for(size_t i=0; i<mat->textures.size(); i++) {
            s >> name;
            mat->textures[i] = texture_load(directory + '/' + name);
        }
        s >> name; assert(name == "amb");
        s >> mat->ambient.x
            >> mat->ambient.y
            >> mat->ambient.z;
        s >> name; assert(name == "dif");
        s >>   mat->diffuse.x
            >> mat->diffuse.y
            >> mat->diffuse.z;
        s >> name; assert(name == "spc");
        s >>   mat->specular.x
            >> mat->specular.y
            >> mat->specular.z;
        s >> name; assert(name == "shn");
        s >>   mat->shininess;
        s >> name; assert(name == "alb");
        s >>   mat->albedo.x
            >> mat->albedo.y
            >> mat->albedo.z;
        s >> name; assert(name == "mtl");
        s >>   mat->metallic;
        s >> name; assert(name == "rgh");
        s >>   mat->roughness;
        s >> name; assert(name == "did");
        s >>   mat->diffuse_id;
        s >> name; assert(name == "sid");
        s >>   mat->specular_id;
        s >> name; assert(name == "mid");
        s >>   mat->metallic_id;
        s >> name; assert(name == "rid");
        s >>   mat->roughness_id;
        s >> name; assert(name == "aid");
        s >>   mat->ao_id;
        s >> name; assert(name == "nid");
        s >>   mat->normal_id;
        meshes[i] = new Mesh(verts, idxs, mat, shader);
    }

    s >> name; assert("bones");
    s >> count;
    if(count > 0) bones.resize(count);
    for(size_t i=0; i<bones.size(); i++) {
        s >> name; assert("b");
        s >> bones[i].name;
        glm::mat4 to;
        s >> to[0].x >> to[0].y >> to[0].z >> to[0].w
            >> to[1].x >> to[1].y >> to[1].z >> to[1].w
            >> to[2].x >> to[2].y >> to[2].z >> to[2].w
            >> to[3].x >> to[3].y >> to[3].z >> to[3].w;
        bones[i].offset = to;
    }
}

}
