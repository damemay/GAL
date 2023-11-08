#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord0;
layout (location = 3) in vec4 joints;
layout (location = 4) in vec4 weights;

out vec2 uv0;

uniform mat4 mvp;

void main() {
    gl_Position = mvp * vec4(position, 1.0);
    uv0 = texcoord0;
}
