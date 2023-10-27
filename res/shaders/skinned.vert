#version 330 core

layout (location = 0) in vec3 pos_in;
layout (location = 1) in vec3 normal_in;
layout (location = 2) in vec2 uv_in;
layout (location = 3) in ivec4 joints;
layout (location = 4) in vec4 weights;

out vec4 pos;
out vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 pose[120];
uniform mat4 inv_bind_pose[120];

void main() {
    mat4 skin =  (pose[joints.x] * inv_bind_pose[joints.x]) * weights.x;
    skin      += (pose[joints.y] * inv_bind_pose[joints.y]) * weights.y;
    skin      += (pose[joints.z] * inv_bind_pose[joints.z]) * weights.z;
    skin      += (pose[joints.w] * inv_bind_pose[joints.w]) * weights.w;
    pos = model*skin*vec4(pos_in, 1.0);
    gl_Position = projection * view * model * skin * vec4(pos_in, 1.0);
    normal = vec3(model*skin*vec4(normal_in, 0.0f));
    // boned_color = vec3(bone_id);
}
