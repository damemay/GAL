#version 330 core

out vec4 out_color;

struct Material {
    vec3 albedo;
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
    vec3 position;
    vec3 direction;

    vec3 color;
};

in vec2 uv0;
in vec3 wpos;
in vec3 norm;

uniform vec3 camera_position;
uniform Material material;
uniform Light light;

const float pi = 3.14159265359;

vec3 fres(float cos_t, vec3 f0) {
    return f0+(1.0-f0)*pow(clamp(1.0-cos_t,0.0,1.0),5.0);
}

float dist_ggx(vec3 n, vec3 h, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float ndoth = max(dot(n,h),0.0);
    float ndoth2 = ndoth*ndoth;
    float num=a2;
    float denom=(ndoth2*(a2-1.0)+1.0);
    denom=pi*denom*denom;
    return num/denom;
}

float geom_sch_ggx(float ndotv, float roughness) {
    float r = (roughness+1.0);
    float k = (r*r)/8.0;
    float num = ndotv;
    float denom = ndotv*(1.0-k)+k;
    return num/denom;
}

float geom_smith(vec3 n, vec3 v, vec3 l, float roughness) {
    float ndotv = max(dot(n,v),0.0);
    float ndotl = max(dot(n,l),0.0);
    float ggx2 = geom_sch_ggx(ndotv, roughness);
    float ggx1 = geom_sch_ggx(ndotl, roughness);
    return ggx1*ggx2;
}

void main() {
    vec3 albedo;
    float metallic;
    float roughness;
    float ao;

    if(material.diffuse_tex_exists) {
        albedo = pow(texture(material.diffuse_tex, uv0).rgb, vec3(2.2));
    } else {
        albedo = material.albedo;
    }

    if(material.metallic_tex_exists) {
        metallic = texture(material.metallic_tex, uv0).r;
    } else {
        metallic = material.metallic;
    }

    if(material.roughness_tex_exists) {
        roughness = texture(material.roughness_tex, uv0).r;
    } else {
        roughness = material.roughness;
    }

    if(material.ao_tex_exists) {
        ao = texture(material.ao_tex, uv0).r;
    } else {
        ao = 1.0f;
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
    vec3 v = normalize(camera_position - wpos);
    vec3 f0 = vec3(0.04);
    f0 = mix(f0, albedo, metallic);

    vec3 lo = vec3(0.0);
    vec3 l = normalize(-light.direction);
    vec3 h = normalize(v+l);
    float dist = length(-light.direction);
    float attent = 1.0/(dist*dist);
    vec3 rad = light.color*attent;
    float ndf = dist_ggx(normal,h,roughness);
    float g = geom_smith(normal,v,l,roughness);
    vec3 f = fres(max(dot(h,v),0.0),f0);
    vec3 ks = f;
    vec3 kd = vec3(1.0)-ks;
    kd *= 1.0-metallic;
    vec3 num = ndf*g*f;
    float denom = 4.0*max(dot(normal,v),0.0)*max(dot(normal,l),0.0)+0.0001;
    vec3 spec = num/denom;
    float ndotl = max(dot(normal,l),0.0);
    lo += (kd*albedo/pi+spec)*rad*ndotl;
    vec3 ambient = vec3(0.03)*albedo*ao;
    vec3 color = ambient+lo;
    color = color/(color+vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    out_color = vec4(color, 1.0);
}
