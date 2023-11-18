struct Material {
    float3 ambient;
    float3 diffuse;
    float3 specular;
    float shininess;

    sampler2D diffuse_tex;
    sampler2D normal_tex;
    sampler2D specular_tex;

    bool diffuse_tex_exists;
    bool normal_tex_exists;
    bool specular_tex_exists;
};

struct Light {
    float3 position;
    float3 direction;

    float3 ambient;
    float3 diffuse;
    float3 specular;

    bool directional;

    float linear;
    float quadratic;
};

struct Fog {
    float3 color;
    float near;
    float far;
};

float lin_depth(Fog fog, float depth) {
    float z = depth*2.0f-1.0f;
    return (2.0f*fog.near*fog.far)/(fog.far+fog.near-z*(fog.far-fog.near));
}

float4 main(
        float2 uv0 : TEXCOORD0,
        float3 wpos : TEXCOORD1,
        float3 norm : TEXCOORD2,
        uniform float3 camera_position,
        uniform Material material,
        uniform Light light,
        uniform Fog fog,
        varying in float4 gl_FragCoord : WPOS
        )
{
    if(material.diffuse_tex_exists) {
        if(tex2D(material.diffuse_tex, uv0).a < 0.1) discard;
    }

    float3 ambient;
    if(material.diffuse_tex_exists) {
        ambient = light.ambient * tex2D(material.diffuse_tex, uv0).rgb;
    } else {
        ambient = light.ambient * material.ambient;
    }

    float3 normal = normalize(norm);

    float3 light_dir;
    if(light.directional) {
        light_dir = normalize(-light.direction);
    } else {
        light_dir = normalize(light.position-wpos);
    }

    float diff = max(dot(normal, light_dir), 0.0f);

    float3 diffuse;
    if(material.diffuse_tex_exists) {
        diffuse = light.diffuse * diff * tex2D(material.diffuse_tex, uv0).rgb;
    } else {
        diffuse = light.diffuse * (diff * material.diffuse);
    }

    float3 view_dir = normalize(camera_position - wpos);
    float3 reflect_dir = reflect(-light_dir, normal);

    float spec;
    if(material.shininess > 0) {
        spec = pow(max(dot(view_dir, reflect_dir), 0.0f), material.shininess);
    } else {
        spec = pow(max(dot(view_dir, reflect_dir), 0.0f), 0);
    }
    float3 specular;
    if(material.specular_tex_exists) {
        specular = light.specular * spec * tex2D(material.specular_tex, uv0).rgb;
    } else if(material.diffuse_tex_exists) {
        specular = light.specular * spec * tex2D(material.diffuse_tex, uv0).rgb;
    } else {
        specular = light.specular * (spec * material.specular);
    }

    if(!light.directional) {
        float dist = length(light.position-wpos);
        float attent = 1.0f/(1.0f+light.linear*dist+light.quadratic*(dist*dist));
        ambient *= attent;
        diffuse *= attent;
        specular *= attent;
    }

    float3 result = ambient + diffuse + specular;
    float fog_factor = lin_depth(fog, gl_FragCoord.z)/fog.far;
    fog_factor = clamp(fog_factor, 0.0f, 1.0f);
    float4 color = lerp(float4(result, 1.0f), float4(fog.color, 1.0f), fog_factor);
    return color;
}
