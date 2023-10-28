#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texcoord0;

//out vec3 normals;
out vec2 uv0;

uniform mat4 vp;
uniform mat4 model;

void main() {
    gl_Position = vp * model * vec4(position, 1.0f);
    //normals = vec3(model*vec4(normal, 0.0f));
    uv0 = texcoord0;
}
