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

float4x4 scale(float4x4 m, float s) {
  float a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3];
  float a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3];
  float a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3];
  float a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3];
  return float4x4(a00*s, a01*s, a02*s, a03*s,
          a10*s, a11*s, a12*s, a13*s,
          a20*s, a21*s, a22*s, a23*s,
          a30*s, a31*s, a32*s, a33*s);
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
    float4x4 skin = scale(pose[(int)(joints.x)], weights.x) +
        scale(pose[(int)(joints.y)], weights.y) +
        scale(pose[(int)(joints.z)], weights.z) +
        scale(pose[(int)(joints.w)], weights.w);
    wpos = float3(mul(model, float4(position, 1.0f)));
    uv0 = texcoord0;
    norm = mul(transpose(inverse(float3x3(model))), normal);
    gl_Position = mul(vp, mul(skin, float4(wpos, 1.0f)));
}
