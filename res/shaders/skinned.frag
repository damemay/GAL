#version 330 core

in vec3 norm;

out vec4 color;

void main() {
    vec3 nnn = normalize(norm);
    color = vec4(nnn, 1.0f);
}
