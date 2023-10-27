void main(
    float3 pos_in,
    float3 normal_in,
    column_major uniform float4x4 vp,
    column_major uniform float4x4 model,
    column_major uniform float4x4 normal_mat,
    float4 out gl_Position : POSITION,
    float3 out normal : TEXCOORD1,
    float3 out frag_pos : TEXCOORD2)
{
    frag_pos = float3(mul(model, float4(pos_in, 1.0f)));
    normal = mul(float3x3(normal_mat), normal_in);
    gl_Position = mul(vp, float4(frag_pos, 1.0f));
}
