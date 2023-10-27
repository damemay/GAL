struct _light {
    float3 direction;
    float3 ambient;
    float3 diffuse;
    float3 specular;
};

struct _material {
    float3 ambient;
    float3 diffuse;
    float3 specular;
    float shininess;
};

float4 main(
    float3 normal : TEXCOORD1,
    float3 frag_pos : TEXCOORD2,
    uniform _material mat,
    uniform _light light,
    uniform float3 view_pos)
{
    float3 ambient = light.ambient * mat.ambient;

    float3 norm = normalize(normal);
    //float3 light_dir = normalize(light.pos - frag_pos);
    float3 light_dir = normalize(-light.direction);
    float diff = max(dot(norm, light_dir), 0.0f);
    float3 diffuse = light.diffuse * (diff * mat.diffuse);

    float3 view_dir = normalize(view_pos - frag_pos);
    float3 refl_dir = reflect(-light_dir, norm);
    float spec = pow(max(dot(view_dir, refl_dir), 0.0f), mat.shininess);
    float3 specular = light.diffuse * (spec * mat.specular);

    float3 final = ambient + diffuse + specular;

    float4 color = float4(final, 1.0f);
    return color;
}
