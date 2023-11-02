#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv0;
layout (location = 3) in vec4 joints;
layout (location = 4) in vec4 weights;

out vec4 pos;
out vec3 norm;

uniform mat4 mvp;

uniform mat4 pose[100];

void main() {
    mat4 skin = weights.x * pose[int(joints.x)] +
                weights.y * pose[int(joints.y)] +
                weights.z * pose[int(joints.z)] +
                weights.w * pose[int(joints.w)];
    norm = vec3(mvp*vec4(normal, 0.0f));
    gl_Position = mvp * skin * vec4(position, 1.0);
}
