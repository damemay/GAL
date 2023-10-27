#version 330 core

// in vec2 uv;
in vec3 pos;
in vec3 normal;
// in vec3 boned_color;

out vec4 color;

// uniform sampler2D tex_diff1;

void main() {
    vec3 nnn = normalize(normal);
    color = vec4(nnn, 1.0f);
    // color = texture(tex_diff1, uv) * vec4(final, 1.0f);
}
