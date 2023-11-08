#version 330 core

in vec2 uv0;

out vec4 color;

uniform sampler2D tex0;

float near = 0.1;
float far = 100.0;

float lin_depth(float depth) {
    float z = depth*2.0-1.0;
    return (2.0*near*far)/(far+near-z*(far-near));
}

void main() {
    vec4 fog_color = vec4(0.4, 0.4, 0.4, 1.0);
    
    float fog_factor = lin_depth(gl_FragCoord.z)/far;
    fog_factor = clamp(fog_factor, 0.0, 1.0);

    color = mix(texture(tex0, uv0), fog_color, fog_factor);
}
