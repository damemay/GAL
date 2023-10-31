#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uv0;
layout (location = 3) in ivec4 joints;
layout (location = 4) in vec4 weights;

out vec4 pos;
out vec3 normal;

uniform mat4 model;
uniform mat4 vp;

uniform mat4 pose[120];

void main() {
    mat4 skin = weights.x * pose[joints.x] +
                weights.y * pose[joints.y] +
                weights.z * pose[joints.z] +
                weights.w * pose[joints.w];
    gl_Position = vp * model * skin * vec4(position, 1.0);
}
