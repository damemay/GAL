#version 330 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 uv_in;

out vec2 uv;
out vec3 frag_pos;
out vec3 normal;

uniform mat4 vp;
uniform mat4 model;
uniform mat4 normal_mat;

void main() {
    frag_pos = vec3(model * vec4(pos_in, 1.0f));
    uv = uv_in;
    normal = vec3(normal_mat);
    gl_Position = vp * vec4(frag_pos, 1.0f);
}
