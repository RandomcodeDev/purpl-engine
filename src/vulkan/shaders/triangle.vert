#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 frag_color;

vec3 verts[3] = vec3[](
	vec3(0.0, -0.5, 0.0),
	vec3(0.5, 0.5, 0.0),
	vec3(-0.5, 0.5, 0.0)
);

vec4 colors[3] = vec4[](
	vec4(1.0, 0.0, 0.0, 1.0),
	vec4(0.0, 1.0, 0.0, 1.0),
	vec4(0.0, 0.0, 1.0, 1.0)
);

void main()
{
	gl_Position = vec4(verts[gl_VertexIndex], 1.0);
	frag_color = colors[gl_VertexIndex];
}
