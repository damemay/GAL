#version 330 core

in vec2 uv;
in vec3 frag_pos;
in vec3 normal;

out vec4 color;

struct _light {
    // vec3 pos;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct _material {
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

uniform _material mat;
uniform _light light;
uniform vec3 view_pos;

uniform sampler2D tex;

void main() {
    vec3 ambient = light.ambient * mat.diffuse;

    vec3 norm = normalize(normal);
    // vec3 l_dir = normalize(light.pos - frag_pos);
    vec3 l_dir = normalize(-light.direction);
    float diff = max(dot(norm, l_dir), 0.0f);
    vec3 diffuse = light.diffuse * diff * mat.diffuse;

    vec3 view_dir = normalize(view_pos - frag_pos);
    vec3 refl_dir = reflect(-l_dir, norm);
    float spec = pow(max(dot(view_dir, refl_dir), 0.0f), mat.shininess);
    vec3 specular = light.specular * spec * mat.specular;

    vec3 final = ambient + diffuse + specular;

    //color = vec4(final, 1.0f);
    color = texture(tex, uv) * vec4(final, 1.0f);
}
