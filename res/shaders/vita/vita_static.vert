void main(
        float3 position,
        float3 __nostrip normal,
        float2 __nostrip texcoord0,
        float4 __nostrip joints,
        float4 __nostrip weights,
        column_major uniform float4x4 mvp,
        float4 out gl_Position : POSITION,
        float2 out uv0 : TEXCOORD0
        )
{
    gl_Position = mul(mvp, float4(position, 1.0f));
    uv0 = texcoord0;
}
