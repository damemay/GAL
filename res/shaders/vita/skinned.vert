float3x3 inverse(float3x3 m) {
  float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2];
  float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2];
  float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2];

  float b01 = a22 * a11 - a12 * a21;
  float b11 = -a22 * a10 + a12 * a20;
  float b21 = a21 * a10 - a11 * a20;

  float det = a00 * b01 + a01 * b11 + a02 * b21;

  return float3x3(b01, (-a22 * a01 + a02 * a21), (a12 * a01 - a02 * a11),
              b11, (a22 * a00 - a02 * a20), (-a12 * a00 + a02 * a10),
              b21, (-a21 * a00 + a01 * a20), (a11 * a00 - a01 * a10)) / det;
}

void main(
        float3 position,
        float3 __nostrip normal,
        float2 __nostrip texcoord0,
        float4 __nostrip joints,
        float4 __nostrip weights,
        column_major uniform float4x4 vp,
        column_major uniform float4x4 model,
        column_major uniform float4x4 pose[100],
        float4 out gl_Position : POSITION,
        float2 out uv0 : TEXCOORD0,
        float3 out wpos : TEXCOORD1,
        float3 out norm : TEXCOORD2)
{
    float3 pos = 0;
    pos += weights.x * mul(pose[(int)(joints.x)], float4(position, 1.0f)).xyz;
    pos += weights.y * mul(pose[(int)(joints.y)], float4(position, 1.0f)).xyz;
    pos += weights.z * mul(pose[(int)(joints.z)], float4(position, 1.0f)).xyz;
    pos += weights.w * mul(pose[(int)(joints.w)], float4(position, 1.0f)).xyz;
    wpos = float3(mul(model, float4(pos, 1.0f)));
    uv0 = texcoord0;
    norm = mul(transpose(inverse(float3x3(model))), normal);
    gl_Position = mul(vp, float4(wpos, 1.0f));
    uv0 = texcoord0;
}
