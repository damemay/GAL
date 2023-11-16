struct Material {
    float3 albedo;
    float metallic;
    float roughness;

    sampler2D diffuse_tex;
    sampler2D normal_tex;
    sampler2D metallic_tex;
    sampler2D roughness_tex;
    sampler2D ao_tex;

    bool diffuse_tex_exists;
    bool normal_tex_exists;
    bool metallic_tex_exists;
    bool roughness_tex_exists;
    bool ao_tex_exists;
};

struct Light {
    float3 position;
    float3 direction;

    float3 color;

    bool directional;
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

static const float pi = 3.14159265359;

float3 fres(float cos_t, float3 f0) {
    return f0+(1.0f-f0)*pow(clamp(1.0f-cos_t,0.0f,1.0f),5.0f);
}

float dist_ggx(float3 n, float3 h, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float ndoth = max(dot(n,h),0.0f);
    float ndoth2 = ndoth*ndoth;
    float num = a2;
    float denom = (ndoth2*(a2-1.0f)+1.0f);
    denom = pi*denom*denom;
    return num/denom;
}

float geom_sch_ggx(float ndotv, float roughness) {
    float r = (roughness+1.0f);
    float k = (r*r)/8.0f;
    float num = ndotv;
    float denom = ndotv*(1.0f-k)+k;
    return num/denom;
}

float geom_smith(float3 n, float3 v, float3 l, float roughness) {
    float ndotv = max(dot(n,v),0.0f);
    float ndotl = max(dot(n,l),0.0f);
    float ggx2 = geom_sch_ggx(ndotv, roughness);
    float ggx1 = geom_sch_ggx(ndotl, roughness);
    return ggx1*ggx2;
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
    if(tex2D(material.diffuse_tex, uv0).a < 0.1) discard;

    float3 albedo;
    float metallic;
    float roughness;
    float ao;

    if(material.diffuse_tex_exists) {
        albedo = pow(tex2D(material.diffuse_tex, uv0).rgb, float3(2.2f, 2.2f, 2.2f));
    } else {
        albedo = material.albedo;
    }

    if(material.metallic_tex_exists) {
        metallic = tex2D(material.metallic_tex, uv0).r;
    } else {
        metallic = material.metallic;
    }

    if(material.roughness_tex_exists) {
        roughness = tex2D(material.roughness_tex, uv0).r;
    } else {
        roughness = material.roughness;
    }

    if(material.ao_tex_exists) {
        ao = tex2D(material.ao_tex, uv0).r;
    } else {
        ao = 1.0f;
    }

    float3 normal = normalize(norm);
    float3 v = normalize(camera_position-wpos);
    float3 f0 = float3(0.04f, 0.04f, 0.04f);
    f0 = lerp(f0, albedo, metallic);

    float3 lo = float3(0.0f, 0.0f, 0.0f);
    float3 l;
    if(light.directional) {
        l = normalize(-light.direction);
    } else {
        l = normalize(light.position-wpos);
    }
    float3 h = normalize(v+l);
    float dist;
    if(light.directional) {
        dist = length(-light.direction);
    } else {
        dist = length(light.position-wpos);
    }
    float attent = 1.0f/(dist*dist);
    float3 rad = light.color*attent;
    float ndf = dist_ggx(normal,h,roughness);
    float g = geom_smith(normal,v,l,roughness);
    float3 f = fres(max(dot(h,v),0.0f),f0);
    float3 ks = f;
    float3 kd = float3(1.0f, 1.0f, 1.0f)-ks;
    kd *= 1.0f-metallic;
    float3 num = ndf*g*f;
    float denom = 4.0f*max(dot(normal,v),0.0f)*max(dot(normal,l),0.0f)+0.0001f;
    float3 spec = num/denom;
    float ndotl = max(dot(normal,l),0.0f);
    lo += (kd*albedo/pi+spec)*rad*ndotl;
    float3 ambient = float3(0.03f, 0.03f, 0.03f)*albedo*ao;
    float3 color = ambient+lo;
    color = color/(color+float3(1.0f, 1.0f, 1.0f));
    color = pow(color, float3(1.0f/2.2f, 1.0f/2.2f, 1.0f/2.2f));

    float fog_factor = lin_depth(fog, gl_FragCoord.z)/fog.far;
    fog_factor = clamp(fog_factor, 0.0f, 1.0f);
    float4 out_color = lerp(float4(color, 1.0f), float4(fog.color, 1.0f), fog_factor);
    return out_color;
}
