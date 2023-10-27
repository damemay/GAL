void main(
    float2 pos,
    float2 uv_in,
    float4 out gl_Position : POSITION,
    float2 out uv : TEXCOORD0)
{
    gl_Position = float4(pos, 0.0f, 1.0f);
    uv = uv_in;
}