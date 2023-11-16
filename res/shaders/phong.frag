#version 330 core

out vec4 color;

struct Material {
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;

    sampler2D diffuse_tex;
    sampler2D normal_tex;
    sampler2D specular_tex;

    bool diffuse_tex_exists;
    bool normal_tex_exists;
    bool specular_tex_exists;
};

struct Light {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    bool directional;

    float linear;
    float quadratic;
};

struct Fog {
    vec3 color;
    float near;
    float far;
};

in vec2 uv0;
in vec3 wpos;
in vec3 norm;

uniform vec3 camera_position;
uniform Material material;
uniform Light light;
uniform Fog fog;

float lin_depth(float depth) {
    float z = depth*2.0-1.0;
    return (2.0*fog.near*fog.far)/(fog.far+fog.near-z*(fog.far-fog.near));
}

void main()
{
    if(texture(material.diffuse_tex, uv0).a < 0.1) discard;

    vec3 ambient;
    if(material.diffuse_tex_exists) {
        ambient = light.ambient * texture(material.diffuse_tex, uv0).rgb;
    } else {
        ambient = light.ambient * material.ambient;
    }
 
    vec3 normal;
    if(material.normal_tex_exists) {
        vec3 tangent = texture(material.normal_tex, uv0).rgb * 2.0 - 1.0;
        vec3 q1 = dFdx(wpos);
        vec3 q2 = dFdy(wpos);
        vec2 st1 = dFdx(uv0);
        vec2 st2 = dFdy(uv0);
        vec3 n = normalize(norm);
        vec3 t = normalize(q1*st2.t-q2*st1.t);
        vec3 b = -normalize(cross(n,t));
        mat3 tbn = mat3(t,b,n);
        normal = normalize(tbn*tangent);
    } else {
        normal = normalize(norm);
    }
    vec3 light_dir;
    if(light.directional) {
        light_dir = normalize(-light.direction);
    } else {
        light_dir = normalize(light.position-wpos);
    }
    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse;
    if(material.diffuse_tex_exists) {
        diffuse = light.diffuse * diff * texture(material.diffuse_tex, uv0).rgb;
    } else {
        diffuse = light.diffuse * (diff * material.diffuse);
    }

    vec3 view_dir = normalize(camera_position - wpos);
    vec3 reflect_dir = reflect(-light_dir, normal);
    float spec;
    if(material.shininess > 0) {
        spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
    } else {
        spec = pow(max(dot(view_dir, reflect_dir), 0.0), 0);
    }
    vec3 specular;
    if(material.specular_tex_exists) {
        specular = light.specular * spec * texture(material.specular_tex, uv0).rgb;
    } else if(material.diffuse_tex_exists) {
        specular = light.specular * spec * texture(material.diffuse_tex, uv0).rgb;
    } else {
        specular = light.specular * (spec * material.specular);
    }

    if(!light.directional) {
        float dist = length(light.position-wpos);
        float attent = 1.0/(1.0+light.linear*dist+light.quadratic*(dist*dist));
        ambient *= attent;
        diffuse *= attent;
        specular *= attent;
    }

    vec3 result = ambient + diffuse + specular;
    float fog_factor = lin_depth(gl_FragCoord.z)/fog.far;
    fog_factor = clamp(fog_factor, 0.0, 1.0);

    color = mix(vec4(result,1.0), vec4(fog.color,1.0), fog_factor);
    //color = vec4(result, 1.0);
}
