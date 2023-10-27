#version 330 core
layout (location = 0) in vec3 pos_in;

out vec3 uv;

uniform mat4 view;
uniform mat4 proj;

void main()
{
    uv = pos_in;
    vec4 pos = proj * mat4(mat3(view)) * vec4(pos_in, 1.0);
    gl_Position = pos.xyww;
}  
