#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord0;
layout (location = 3) in vec4 joints;
layout (location = 4) in vec4 weights;

out vec2 uv0;
out vec3 wpos;
out vec3 norm;

uniform mat4 vp;
uniform mat4 model;
uniform mat4 pose[100];

void main() {
    mat4 skin = weights.x * pose[int(joints.x)] +
                weights.y * pose[int(joints.y)] +
                weights.z * pose[int(joints.z)] +
                weights.w * pose[int(joints.w)];
    uv0 = texcoord0;
    wpos = vec3(model * vec4(position, 1.0));
    norm = transpose(inverse(mat3(model)))*normal;
    gl_Position = vp * skin * vec4(wpos, 1.0);
    uv0 = texcoord0;
}
