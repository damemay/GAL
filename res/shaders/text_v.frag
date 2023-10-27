float4 main(
    float4 gl_FragColor : COLOR,
    float2 uv : TEXCOORD0,
    uniform sampler2D tex
) {
    gl_FragColor = tex2D(tex, float2(uv.x, uv.y));
    return gl_FragColor;
}