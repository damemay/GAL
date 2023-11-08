float4 main (
        float2 uv0: TEXCOORD0,
        uniform sampler2D tex)
{
    float4 color = tex2D(tex, uv0);
    return color;
}
