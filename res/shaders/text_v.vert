void main(
    float2 pos_ss,
    float2 uv_in,
    float4 out gl_Position : POSITION,
    float2 out uv : TEXCOORD0)
{
    float2 pos = pos_ss - float2(480, 272);
    pos /= float2(480, 272);
    gl_Position = float4(pos, 0.0, 1.0);
    uv = uv_in;
}