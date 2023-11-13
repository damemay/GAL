void main(
    float2 position,
    float2 uv_in,
    float4 out gl_Position : POSITION,
    float2 out uv : TEXCOORD0)
{
    gl_Position = float4(position, 0.0, 1.0);
    uv = uv_in;
}
