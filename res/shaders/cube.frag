#version 330 core

//in vec3 normals;
in vec2 uv0;

out vec4 color;

uniform sampler2D tex0;

void main() {
    // vec3 nnn = normalize(normals);
    color = texture(tex0, uv0);
}
