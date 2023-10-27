#version 330 core
layout(location = 0) in vec2 pos_ss;
layout(location = 1) in vec2 uv_in;

out vec2 uv;

void main() {
    vec2 pos = pos_ss - vec2(480, 272);
    pos /= vec2(480, 272);
    gl_Position = vec4(pos, 0.0f, 1.0f);
    uv = uv_in;
}