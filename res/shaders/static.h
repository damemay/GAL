const unsigned char static_vert[] =
	"#version 330 core\n\nlayout (location = 0) in vec3 position;\nlayout (location = 1) in vec3 normal;\n"
	"layout (location = 2) in vec2 texcoord0;\nlayout (location = 3) in vec4 joints;\nlayout (location = "
	"4) in vec4 weights;\n\nout vec2 uv0;\nout vec3 wpos;\nout vec3 norm;\n\nuniform mat4 vp;\nuniform ma"
	"t4 model;\n\nvoid main() {\n    uv0 = texcoord0;\n    wpos = vec3(model * vec4(position, 1.0));\n   "
	" norm = transpose(inverse(mat3(model)))*normal;\n    gl_Position = vp * vec4(wpos, 1.0);\n}\n";
const unsigned int static_vert_len = 473;
