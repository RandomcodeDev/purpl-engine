#version 460
#extension GL_ARB_separate_shader_objects : enable

vec4 verts[] = vec4[](
	vec4(0.0, -0.5, 0.0, 1.0),
	vec4(0.5, 0.5, 0.0, 1.0),
	vec4(-0.5, 0.5, 0.0, 1.0)
);

vec4 colors[] = vec4[](
	vec4(1.0, 0.0, 0.0, 1.0),
	vec4(0.0, 1.0, 0.0, 1.0),
	vec4(0.0, 0.0, 1.0, 1.0)
);

layout(location = 0) out vec4 out_color;

void main()
{
	gl_Position = verts[gl_VertexIndex];
	out_color = colors[gl_VertexIndex];
}
