#version 330 core

layout(location = 0) in vec2 pos;
layout(location = 1) in vec2 uv_in;

out vec2 uv;

void main() {
    gl_Position = vec4(pos, 0.0f, 1.0f);
    uv = uv_in;
}