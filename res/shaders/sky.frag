#version 330 core

in vec2 uv;
out vec4 out_color;
uniform mat4 view;
uniform sampler2D tex;

void main() {
    vec3 v = vec3(view);
    // v = normalize(v);
    out_color = texture(tex, vec2(
                (uv.x/6)+smoothstep(-1.0f, 1.0f, v.z)/2,
                -1.0f*(uv.y/1.5)));
}
