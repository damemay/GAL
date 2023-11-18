const unsigned char skinned_vert[] =
	"#version 330 core\n\nlayout (location = 0) in vec3 position;\nlayout (location = 1) in vec3 normal;\n"
	"layout (location = 2) in vec2 texcoord0;\nlayout (location = 3) in vec4 joints;\nlayout (location = "
	"4) in vec4 weights;\n\nout vec2 uv0;\nout vec3 wpos;\nout vec3 norm;\n\nuniform mat4 vp;\nuniform ma"
	"t4 model;\nuniform mat4 pose[100];\n\nvoid main() {\n    mat4 skin = weights.x * pose[int(joints.x)]"
	" +\n                weights.y * pose[int(joints.y)] +\n                weights.z * pose[int(joints.z"
	")] +\n                weights.w * pose[int(joints.w)];\n    uv0 = texcoord0;\n    wpos = vec3(model "
	"* vec4(position, 1.0));\n    norm = transpose(inverse(mat3(model)))*normal;\n    gl_Position = vp * "
	"skin * vec4(wpos, 1.0);\n}\n";
const unsigned int skinned_vert_len = 703;
