void main(
        float3 position,
        float3 __nostrip normal,
        float2 __nostrip texcoord0,
        float4 __nostrip joints,
        float4 __nostrip weights,
        column_major uniform float4x4 mvp,
        column_major uniform float4x4 pose[100],
        float4 out gl_Position : POSITION,
        float2 out uv0 : TEXCOORD0
        )
{
    int4 joint = (int4)joints;
    float4x4 skin = weights.x * pose[joint.x] +
                weights.y * pose[joint.y] +
                weights.z * pose[joint.z] +
                weights.w * pose[joint.w];
    gl_Position = mul(mvp, mul(skin, float4(position, 1.0f)));
    uv0 = texcoord0;
}
